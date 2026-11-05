/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio
    ChibiOS - Copyright (C) 2023..2024 HorrorTroll
    ChibiOS - Copyright (C) 2023..2024 Zhaqian

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    USBOTGv1/hal_usb_lld.c
 * @brief   N32 USB subsystem low level driver source.
 *
 * @addtogroup USB
 * @{
 */

#include <string.h>

#include "hal.h"

#if HAL_USE_USB || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define USBTRDTIM_VALUE_FS      5
#define USBTRDTIM_VALUE_HS      9

#define EP0_MAX_INSIZE          64
#define EP0_MAX_OUTSIZE         64

/* N32H48x USBHS clock enable/reset helpers. The USBHS is on the AHB bus.
   NOTE: the USBHS PHY needs an explicit clock source and frequency
   selection in RCC_USBHSCTRL1 -- without it the default is HSE, which is
   not enabled on this board (the port runs from HSI + PLL), so the PHY
   would never lock and USB would not enumerate.

   Clock chain used here (the SDK's "PLLPRES" path, adapted to the
   HSI-based 240MHz system PLL):
     system PLL (240MHz) -> RCC_CFG3.USBPLLPRES /10 -> 24MHz
     USBHSCTRL1.USBHSSEL = 1      : clock source = that PLL prescaler output
     USBHSCTRL1.USBHSFSEL = 24MHz : tell the PHY the input frequency
     USBHSCTRL1.PLLBW    = 1      : bandwidth setting for != 16MHz input

   IMPORTANT: the prescaler that feeds USBHSCTRL1.USBHSSEL=1 lives in
   RCC_CFG3 (USBPLLSEL / USBPLLPRES), NOT in RCC_CFG2 (ADCOTGPLL*), which
   is the ADC/OTG-PLL block.  Feeding the PHY from CFG2 left the USBHS
   clock mux un-driven, so the PHY never locked and the host saw nothing.
*/
static void usb_clock_enable(void) {
  /* Select the system PLL as the USB-clock source and divide it by 10:
     240MHz / 10 = 24MHz (a valid USBHS FSEL frequency). */
  RCC->CFG3 &= ~(RCC_CFG3_USBPLLSEL | RCC_CFG3_USBPLLPRES);
  RCC->CFG3 |= RCC_CFG3_USBPLLPRES_DIV10;   /* USBPLLSEL=0 -> system PLL */

  /* USBHS PHY clock source / frequency / bandwidth. */
  RCC->USBHSCTRL1 = (RCC->USBHSCTRL1 &
                     ~(RCC_USBHSCTRL1_USBHSFSEL | RCC_USBHSCTRL1_PLLBW |
                       RCC_USBHSCTRL1_USBHSSEL)) |
                    RCC_USBHSCTRL1_USBHSSEL |    /* source: PLL prescaler  */
                    RCC_USBHSCTRL1_USBHSFSEL_1 | /* FSEL: 24MHz            */
                    RCC_USBHSCTRL1_PLLBW;        /* BW: other than 16MHz   */

  /* PHY transmitter tuning (from the SDK USBHS device examples). */
  RCC->USBHSCTRL2 = (RCC->USBHSCTRL2 &
                     ~((0xFU << 4) | (0x3U << 12) | (0x3U << 14) | (0x3U << 16))) |
                    (0x5U << 4) |   /* TX vref TUNE   */
                    (0x3U << 12) |  /* TX rise TUNE   */
                    (0x3U << 14) |  /* TX res TUNE    */
                    (0x3U << 16);   /* TX preemp TUNE */

  /* Device-mode PHY enable, as the SDK does in USB_SetCurrentMode()
     (DEVICE_MODE branch): IDDQEN=1 and the bit-31 transceiver-path
     select must be set, otherwise the built-in PHY is not in device
     operation and the host never sees the device. */
  RCC->USBHSCTRL2 |= RCC_USBHSCTRL2_IDDQEN;
  RCC->USBHSCTRL1 |= ((uint32_t)0x01U << 31);

  /* Reset the USBHS PHY, then enable the peripheral clock. */
  RCC->AHBPRST |= RCC_AHBPRST_USBHSPHYRST;
  RCC->AHBPRST &= ~RCC_AHBPRST_USBHSPHYRST;
  RCC->AHBPCLKEN |= RCC_AHBPCLKEN_USBHSEN;
}

static void usb_clock_disable(void) {
  RCC->AHBPCLKEN &= ~RCC_AHBPCLKEN_USBHSEN;
}

static void usb_clock_reset(void) {
  RCC->AHBPRST |= RCC_AHBPRST_USBHSRST;
  RCC->AHBPRST &= ~RCC_AHBPRST_USBHSRST;
}

#define IRQ_RETRY_MASK (GINTSTS_NPTXFEMP | GINTSTS_PTXFEMP | GINTSTS_RXFLVL)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USBHS driver identifier.*/
#if N32_USB_USE_OTG1 || defined(__DOXYGEN__)
USBDriver USBD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   EP0 state.
 * @note    It is an union because IN and OUT endpoints are never used at the
 *          same time for EP0.
 */
static union {
  /**
   * @brief   IN EP0 state.
   */
  USBInEndpointState in;
  /**
   * @brief   OUT EP0 state.
   */
  USBOutEndpointState out;
} ep0_state;

/**
 * @brief   Buffer for the EP0 setup packets.
 */
static uint8_t ep0setup_buffer[8];

/**
 * @brief   EP0 initialization structure.
 */
static const USBEndpointConfig ep0config = {
  USB_EP_MODE_TYPE_CTRL,
  _usb_ep0setup,
  _usb_ep0in,
  _usb_ep0out,
  0x40,
  0x40,
  &ep0_state.in,
  &ep0_state.out,
  1,
  ep0setup_buffer
};

#if N32_USB_USE_OTG1
static const n32_otg_params_t hsparams = {
  N32_USB_OTG1_RX_FIFO_SIZE / 4,
  N32_USBHS_FIFO_MEM_SIZE,
  N32_USBHS_ENDPOINTS
};
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void otg_core_reset(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;
  unsigned count;

  /* Wait AHB idle condition, with a bounded timeout so a missing PHY
     clock cannot hang the boot (SDK uses a 200000-iteration limit).*/
  count = 0;
  while ((otgp->GRSTCTL & GRSTCTL_AHBIDLE) == 0) {
    if (++count > 200000)
      return;
  }

  /* Core reset.  N32 (DWC-style) core does not self-clear CSRST, so
     follow the SDK (USB_CoreReset): OR-in CSRST, wait for SRSTDNE and
     then clear CSRST manually.  Writing the bare CSRST value would also
     clear the AHBIDLE status bit and leave the core stuck in reset. */
  otgp->GRSTCTL |= GRSTCTL_CSFTRST;
  osalSysPolledDelayX(12);
  count = 0;
  while ((otgp->GRSTCTL & GRSTCTL_SRSTDNE) == 0) {
    if (++count > 200000)
      break;
  }
  otgp->GRSTCTL &= ~GRSTCTL_CSFTRST;

  osalSysPolledDelayX(18);

  /* Wait AHB idle condition again.*/
  count = 0;
  while ((otgp->GRSTCTL & GRSTCTL_AHBIDLE) == 0) {
    if (++count > 200000)
      return;
  }
}

static void otg_disable_ep(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;
  unsigned i;

  for (i = 0; i <= usbp->otgparams->num_endpoints; i++) {

    if ((otgp->ie[i].DIEPCTL & DIEPCTL_EPTENA) != 0U) {
      otgp->ie[i].DIEPCTL |= DIEPCTL_EPTDIS;
    }

    if ((otgp->oe[i].DOEPCTL & DIEPCTL_EPTENA) != 0U) {
      otgp->oe[i].DOEPCTL |= DIEPCTL_EPTDIS;
    }

    otgp->ie[i].DIEPINT = 0xFFFFFFFF;
    otgp->oe[i].DOEPINT = 0xFFFFFFFF;
  }
  otgp->DAINTMSK = DAINTMSK_OUTEPTMSK(0) | DAINTMSK_INEPTMSK(0);
}

static void otg_enable_ep(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;
  unsigned i;

  for (i = 0; i <= usbp->otgparams->num_endpoints; i++) {
    if (usbp->epc[i]->out_state != NULL) {
      otgp->DAINTMSK |= DAINTMSK_OUTEPTMSK(i);
    }
    if (usbp->epc[i]->in_state != NULL) {
      otgp->DAINTMSK |= DAINTMSK_INEPTMSK(i);
    }
  }
}

static void otg_rxfifo_flush(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;

  otgp->GRSTCTL = GRSTCTL_RXFFLSH;
  while ((otgp->GRSTCTL & GRSTCTL_RXFFLSH) != 0)
    ;
  /* Wait for 3 PHY Clocks.*/
  osalSysPolledDelayX(18);
}

static void otg_txfifo_flush(USBDriver *usbp, uint32_t fifo) {
  n32_otg_t *otgp = usbp->otg;

  otgp->GRSTCTL = GRSTCTL_TXFNUM(fifo) | GRSTCTL_TXFFLSH;
  while ((otgp->GRSTCTL & GRSTCTL_TXFFLSH) != 0)
    ;
  /* Wait for 3 PHY Clocks.*/
  osalSysPolledDelayX(18);
}

/**
 * @brief   Resets the FIFO RAM memory allocator.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
static void otg_ram_reset(USBDriver *usbp) {

  usbp->pmnext = usbp->otgparams->rx_fifo_size;
}

/**
 * @brief   Allocates a block from the FIFO RAM memory.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] size      size of the packet buffer to allocate in words
 *
 * @notapi
 */
static uint32_t otg_ram_alloc(USBDriver *usbp, size_t size) {
  uint32_t next;

  next = usbp->pmnext;
  usbp->pmnext += size;
  osalDbgAssert(usbp->pmnext <= usbp->otgparams->otg_ram_size,
                "OTG FIFO memory overflow");
  return next;
}

/**
 * @brief   Writes to a TX FIFO.
 *
 * @param[in] fifop     pointer to the FIFO register
 * @param[in] buf       buffer where to copy the endpoint data
 * @param[in] n         maximum number of bytes to copy
 *
 * @notapi
 */
static void otg_fifo_write_from_buffer(volatile uint32_t *fifop,
                                       const uint8_t *buf,
                                       size_t n) {

  osalDbgAssert(n > 0, "is zero");

  while (true) {
    *fifop = *((uint32_t *)buf);
    if (n <= 4) {
      break;
    }
    n -= 4;
    buf += 4;
  }
}

/**
 * @brief   Reads a packet from the RXFIFO.
 *
 * @param[in] fifop     pointer to the FIFO register
 * @param[out] buf      buffer where to copy the endpoint data
 * @param[in] n         number of bytes to pull from the FIFO
 * @param[in] max       number of bytes to copy into the buffer
 *
 * @notapi
 */
static void otg_fifo_read_to_buffer(volatile uint32_t *fifop,
                                    uint8_t *buf,
                                    size_t n,
                                    size_t max) {
  uint32_t w = 0;
  size_t i = 0;

  while (i < n) {
    if ((i & 3) == 0) {
      w = *fifop;
    }
    if (i < max) {
      *buf++ = (uint8_t)w;
      w >>= 8;
    }
    i++;
  }
}

/* Instrumentation for Vial debugging (defined non-static so the keyboard
   layer can read them over the UART debug port). */
volatile uint32_t n32dbg_rx_ring[16];
volatile uint32_t n32dbg_rx_ring_i;
volatile uint32_t n32dbg_poll_cnt;
volatile uint32_t n32dbg_poll_ep;
volatile uint32_t n32dbg_poll_val;
/* Which endpoint numbers were ever initialized with an OUT state, and which
   were ever armed for reception (bit N = endpoint N). */
volatile uint32_t n32dbg_initep_out_mask;
volatile uint32_t n32dbg_startout_mask;
/* Cumulative RX status queue taxonomy over ALL pops, plus captured SETUP
   request bytes and the OR of every masked GINTSTS in the ISR. */
volatile uint32_t n32dbg_pop_gen;
volatile uint32_t n32dbg_pop_pk[8];   /* index = PKTSTS value 0..7 */
volatile uint32_t n32dbg_pop_ep_or;   /* OR of (1 << EPTNUM) seen */
volatile uint32_t n32dbg_setup_cnt;
volatile uint32_t n32dbg_setup_buf[2];/* 8 bytes of last SETUP */
volatile uint32_t n32dbg_isr_sts_or;  /* OR of masked GINTSTS over all ISR runs */
/* IN-direction counters: how often the stack arms an IN transfer, how often
   EP0 IN completes (XFERC) and how often the TX-FIFO-empty handler runs. */
volatile uint32_t n32dbg_startin_cnt;
volatile uint32_t n32dbg_startin_ep;
volatile uint32_t n32dbg_startin_sz;
volatile uint32_t n32dbg_epin_xfrc_cnt;
volatile uint32_t n32dbg_epin_xfrc_ep;
volatile uint32_t n32dbg_epin_txfe_cnt;
volatile uint32_t n32dbg_epin_txfe_ep;
/* IN-endpoint polling path (DAINT does not report data endpoints on this
   part, so DIEPINTn is walked from the ISR - see usb_lld_serve_interrupt). */
volatile uint32_t n32dbg_epin_poll_cnt;
volatile uint32_t n32dbg_epin_poll_ep;
volatile uint32_t n32dbg_epin_poll_val;
/* Ring of the last 8 captured SETUP requests (word 0 and word 1 use the same
   byte interleave as n32dbg_setup_buf, so the decoder is identical).
   Reading the whole ring after the freeze tells us which requests the host
   was repeating when the bus went silent. */
volatile uint32_t n32dbg_setup_ring[8][2];
volatile uint32_t n32dbg_setup_ring_i;
/* Enumeration milestones. */
volatile uint32_t n32dbg_setaddr_cnt;   /* usb_lld_set_address() calls */
volatile uint32_t n32dbg_cfg_seen;      /* SET_CONFIGURATION requests seen */
volatile uint32_t n32dbg_stall_in_cnt;
volatile uint32_t n32dbg_stall_in_ep;
volatile uint32_t n32dbg_stall_out_cnt;
volatile uint32_t n32dbg_stall_out_ep;
/* Snapshot of the peripheral state at probe time (filled by
   n32dbg_usb_probe(), called from the keyboard debug dump). */
/* OUT endpoint re-arm bookkeeping.  On a DWC-style core writing DOEPTSIZ while
   EPENA is still set is ignored, so usb_lld_start_out() must disable the
   endpoint first.  These counters prove whether that actually happened. */
volatile uint32_t n32dbg_out_epena_busy;  /* start_out found EPENA already 1 */
volatile uint32_t n32dbg_out_epdis_wait;  /* EPDIS polls spent waiting       */
volatile uint32_t n32dbg_out_epdis_tmo;   /* EPDIS never completed           */
volatile uint32_t n32dbg_out_arm_ep;      /* last endpoint armed             */
volatile uint32_t n32dbg_out_arm_ctl;     /* DOEPCTL value written by arm    */
volatile uint32_t n32dbg_out_arm_siz;     /* DOEPTSIZ value written by arm   */

/* Per-endpoint OUT interrupt raw capture: DOEPINT of EP1..EP8 sampled in the
   ISR, so a NAK/error bit shows up even when nothing else fires. */
volatile uint32_t n32dbg_doepint_snapshot[9];
volatile uint32_t n32dbg_doepint_or;      /* OR of every snapshot taken      */

/* NAK / packet-error capture on the OUT data endpoints: proves whether the
   host really polled us (NAKIF) or nothing ever arrived. */
volatile uint32_t n32dbg_out_nak_cnt;
volatile uint32_t n32dbg_out_nak_ep;
volatile uint32_t n32dbg_out_err_cnt;
volatile uint32_t n32dbg_out_err_ep;
volatile uint32_t n32dbg_out_err_val;

/* Periodic re-arm experiment on the RAW OUT endpoint (EP3), driven from the
   keyboard debug dump: counts rewrites of the transfer descriptor and records
   the resulting control register value. */
volatile uint32_t n32dbg_rearm_cnt;
volatile uint32_t n32dbg_rearm_ctl;
volatile uint32_t n32dbg_ep3_mode;

volatile uint32_t n32dbg_dsts;
volatile uint32_t n32dbg_usb_state;
volatile uint32_t n32dbg_primask;
volatile uint32_t n32dbg_usb_irq_en;
volatile uint32_t n32dbg_usb_cfg_val;    /* usbp->configuration */
volatile uint32_t n32dbg_usb_addr_val;   /* usbp->address */
volatile uint32_t n32dbg_usb_ep0state;   /* EP0 state machine position */

/**
 * @brief   Snapshot the USB interrupt / peripheral liveness state.
 *
 * @details Reads the values that distinguish "the bus went silent" from
 *          "the device stopped taking interrupts": DSTS tells whether the
 *          port is still active (SOFs) or suspended; PRIMASK and the NVIC
 *          enable bit tell whether the USB ISR can still fire at all.
 *
 * @notapi
 */
void n32dbg_usb_probe(void) {
  unsigned i;

  n32dbg_primask = __get_PRIMASK();
  /* NOTE: do NOT read NVIC->ISER[USB_HS_IRQn >> 5].
     USB_HS_IRQn is 116 on this part, so the index is 3, but the Cortex-M4
     NVIC implements only ISER[0..2] (IRQs 0..95).  Reading ISER[3] faults
     with IBUSERR at 0xE000EDF8 - that hard fault killed the firmware and
     made the device disappear from the bus entirely.  The NVIC enable state
     is therefore not sampled; PRIMASK plus DSTS are enough to tell whether
     the interrupt path is alive. */
  n32dbg_usb_irq_en = 0U;

  /* The debug dump runs from the matrix-scan hook, which can fire BEFORE
     usb_lld_init() has assigned USBD1.otg.  Dereferencing a NULL driver
     pointer there is a hard fault (and it took the whole firmware down:
     enumeration finished, then no UART output at all).  Fall back to the
     fixed peripheral base until the driver is initialised. */
  n32_otg_t *otgp = (USBD1.otg != NULL) ? USBD1.otg : N32_USBHS_BASE;

  n32dbg_dsts      = otgp->DSTS;
  n32dbg_usb_state = (uint32_t)USBD1.state;
  n32dbg_usb_cfg_val  = (uint32_t)USBD1.configuration;
  n32dbg_usb_addr_val = (uint32_t)USBD1.address;
  n32dbg_usb_ep0state = (uint32_t)USBD1.ep0state;

  /* Sample every OUT endpoint's status register from thread context.  This
     keeps working even when the USB interrupt has stopped firing, so it is
     the only reliable way to see whether the core ever observed the host
     touching a data endpoint (XFERC = 0x1, NAKIF = 0x2000, error bits). */
  if (USBD1.otgparams != NULL) {
    for (i = 1U; i <= (unsigned)USBD1.otgparams->num_endpoints && i < 9U; i++) {
      uint32_t v = otgp->oe[i].DOEPINT;
      n32dbg_doepint_snapshot[i] = v;
      n32dbg_doepint_or |= v;
    }
  }
}

/**
 * @brief   Incoming packets handler.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
static void otg_rxfifo_handler(USBDriver *usbp) {
  uint32_t sts, ep;
  size_t n, max;

  /* Popping the event word out of the RX FIFO.*/
  sts = usbp->otg->GRXSTSP;

  /* Cumulative taxonomy of every popped status word (not just the ring):
     total pops, per-PKTSTS histogram, OR of endpoint numbers, and the last
     SETUP request bytes.  Together with the ring these prove whether the
     core is really pushing events or the pop pointer is stuck on a stale
     word (symptom: endless identical EP0 OUT_COMP entries).*/
  n32dbg_pop_gen++;
 {
  uint32_t pk = (sts & GRXSTSP_PKTSTS_MASK) >> USBHS_GRXSTS_PKTSTS_POS;
  n32dbg_pop_pk[pk & 7U]++;
  n32dbg_pop_ep_or |= (1U << (sts & GRXSTSP_EPTNUM_MASK));
 }

  /* Debug capture of the raw RX status queue entries (decoded by the
     keyboard layer over the UART debug port).*/
  n32dbg_rx_ring[n32dbg_rx_ring_i] = sts;
  n32dbg_rx_ring_i = (n32dbg_rx_ring_i + 1U) & 15U;

  /* Event details.*/
  n  = (size_t)((sts & GRXSTSP_BCNT_MASK) >> GRXSTSP_BCNT_OFF);
  ep = (sts & GRXSTSP_EPTNUM_MASK) >> GRXSTSP_EPTNUM_OFF;

  switch (sts & GRXSTSP_PKTSTS_MASK) {
  case GRXSTSP_SETUP_DATA:
    otg_fifo_read_to_buffer(usbp->otg->FIFO[0], usbp->epc[ep]->setup_buf,
                            n, 8);
    n32dbg_setup_cnt++;
    n32dbg_setup_buf[0] = ((uint32_t)usbp->epc[ep]->setup_buf[1] << 24U) |
                          ((uint32_t)usbp->epc[ep]->setup_buf[3] << 16U) |
                          ((uint32_t)usbp->epc[ep]->setup_buf[5] << 8U)  |
                           (uint32_t)usbp->epc[ep]->setup_buf[7];
    n32dbg_setup_buf[1] = ((uint32_t)usbp->epc[ep]->setup_buf[0] << 24U) |
                          ((uint32_t)usbp->epc[ep]->setup_buf[2] << 16U) |
                          ((uint32_t)usbp->epc[ep]->setup_buf[4] << 8U)  |
                           (uint32_t)usbp->epc[ep]->setup_buf[6];
    /* Append to the 8-deep SETUP ring so the last requests of a dead
       enumeration can be read back. */
    {
      uint32_t ri = n32dbg_setup_ring_i & 7U;
      n32dbg_setup_ring[ri][0] = n32dbg_setup_buf[0];
      n32dbg_setup_ring[ri][1] = n32dbg_setup_buf[1];
      n32dbg_setup_ring_i = (ri + 1U) & 7U;
    }
    /* Count SET_CONFIGURATION (standard, host-to-device, device recipient). */
    if ((usbp->epc[ep]->setup_buf[0] == 0x00U) &&
        (usbp->epc[ep]->setup_buf[1] == 0x09U)) {
      n32dbg_cfg_seen++;
    }
    break;
  case GRXSTSP_SETUP_COMP:
    break;
  case GRXSTSP_OUT_DATA:
    max = usbp->epc[ep]->out_state->rxsize - usbp->epc[ep]->out_state->rxcnt;
    otg_fifo_read_to_buffer(usbp->otg->FIFO[0],
                            usbp->epc[ep]->out_state->rxbuf,
                            n, max);
    if (n < max) {
      usbp->epc[ep]->out_state->rxbuf += n;
      usbp->epc[ep]->out_state->rxcnt += n;
    }
    else {
      usbp->epc[ep]->out_state->rxbuf += max;
      usbp->epc[ep]->out_state->rxcnt += max;
    }
    break;
  case GRXSTSP_OUT_COMP:
    break;
  case GRXSTSP_OUT_GLOBAL_NAK:
    break;
  default:
    break;
  }
}

/**
 * @brief   Outgoing packets handler.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
static bool otg_txfifo_handler(USBDriver *usbp, usbep_t ep) {

  /* The TXFIFO is filled until there is space and data to be transmitted.*/
  while (true) {
    uint32_t n;

    /* Transaction end condition.*/
    if (usbp->epc[ep]->in_state->txcnt >= usbp->epc[ep]->in_state->txsize) {
      usbp->otg->DIEPEMPMSK &= ~DIEPEMPMSK_INEPTXFEMSK(ep);
      return true;
    }

    /* Number of bytes remaining in current transaction.*/
    n = usbp->epc[ep]->in_state->txsize - usbp->epc[ep]->in_state->txcnt;
    if (n > usbp->epc[ep]->in_maxsize)
      n = usbp->epc[ep]->in_maxsize;

    /* Checks if in the TXFIFO there is enough space to accommodate the
       next packet.*/
    if (((usbp->otg->ie[ep].DTXFSTS & DTXFSTS_INEPTXFSAV_MASK) * 4) < n)
      return false;

#if N32_USB_OTGFIFO_FILL_BASEPRI
    __set_BASEPRI(CORTEX_PRIO_MASK(N32_USB_OTGFIFO_FILL_BASEPRI));
#endif
    otg_fifo_write_from_buffer(usbp->otg->FIFO[ep],
                               usbp->epc[ep]->in_state->txbuf,
                               n);
    usbp->epc[ep]->in_state->txbuf += n;
    usbp->epc[ep]->in_state->txcnt += n;
#if N32_USB_OTGFIFO_FILL_BASEPRI
    __set_BASEPRI(0);
#endif
  }
}

/**
 * @brief   Generic endpoint IN handler.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
static void otg_epin_handler(USBDriver *usbp, usbep_t ep) {
  n32_otg_t *otgp = usbp->otg;
  uint32_t epint = otgp->ie[ep].DIEPINT;

  otgp->ie[ep].DIEPINT = epint;

  if (epint & DIEPINT_TIMEOUT) {
    /* Timeouts not handled yet, not sure how to handle.*/
  }
  if ((epint & DIEPINT_XFERC) && (otgp->DIEPMSK & DIEPMSK_XFERCMSK)) {
    n32dbg_epin_xfrc_cnt++;
    n32dbg_epin_xfrc_ep = ep;
    /* Transmit transfer complete.*/
    USBInEndpointState *isp = usbp->epc[ep]->in_state;

    if (isp->txsize < isp->totsize) {
      /* In case the transaction covered only part of the total transfer
         then another transaction is immediately started in order to
         cover the remaining.*/
      isp->txsize = isp->totsize - isp->txsize;
      isp->txcnt  = 0;
      osalSysLockFromISR();
      usb_lld_start_in(usbp, ep);
      osalSysUnlockFromISR();
    }
    else {
      /* End on IN transfer.*/
      _usb_isr_invoke_in_cb(usbp, ep);
    }
  }
  if ((epint & DIEPINT_TXFEMP) &&
      (otgp->DIEPEMPMSK & DIEPEMPMSK_INEPTXFEMSK(ep))) {
    n32dbg_epin_txfe_cnt++;
    n32dbg_epin_txfe_ep = ep;
    /* TX FIFO empty or emptying.*/
    otg_txfifo_handler(usbp, ep);
  }
}

/**
 * @brief   Generic endpoint OUT handler.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
static void otg_epout_handler(USBDriver *usbp, usbep_t ep) {
  n32_otg_t *otgp = usbp->otg;
  uint32_t epint;

  /* Defensive guard for the polling path below: skip endpoint numbers that
     have no OUT state registered.*/
  if ((ep > usbp->otgparams->num_endpoints) || (usbp->epc[ep] == NULL) ||
      (usbp->epc[ep]->out_state == NULL)) {
    return;
  }

  epint = otgp->oe[ep].DOEPINT;

  /* Diagnostic: record whether the core saw the host poll this endpoint.
     NAKIF means "a host OUT arrived and we answered NAK" - which proves the
     host really is sending and the endpoint's transfer descriptor is wrong.
     Packet/error bits cover the other failure modes. */
  if (epint & DOEPINT_NAKIF) {
    n32dbg_out_nak_cnt++;
    n32dbg_out_nak_ep = ep;
  }
  if (epint & (DOEPINT_OUTPKTERR | DOEPINT_B2BSTUP)) {
    n32dbg_out_err_cnt++;
    n32dbg_out_err_ep  = ep;
    n32dbg_out_err_val = epint;
  }

  /* Resets all EP IRQ sources.*/
  otgp->oe[ep].DOEPINT = epint;

  if ((epint & DOEPINT_SETUP) && (otgp->DOEPMSK & DOEPMSK_SETUPMSK)) {
    /* Setup packets handling, setup packets are handled using a
       specific callback.*/
    _usb_isr_invoke_setup_cb(usbp, ep);
  }

  if ((epint & DOEPINT_XFERC) && (otgp->DOEPMSK & DOEPMSK_XFERCMSK)) {
    USBOutEndpointState *osp;

    /* OUT state structure pointer for this endpoint.*/
    osp = usbp->epc[ep]->out_state;

    /* EP0 requires special handling.*/
    if (ep == 0) {
      /* In case the transaction covered only part of the total transfer
         then another transaction is immediately started in order to
         cover the remaining.*/
      if (((osp->rxcnt % usbp->epc[ep]->out_maxsize) == 0) &&
          (osp->rxsize < osp->totsize)) {
        osp->rxsize = osp->totsize - osp->rxsize;
        osp->rxcnt  = 0;
        osalSysLockFromISR();
        usb_lld_start_out(usbp, ep);
        osalSysUnlockFromISR();
        return;
      }
    }

    /* End on OUT transfer.*/
    _usb_isr_invoke_out_cb(usbp, ep);
  }
}

/**
 * @brief   Poll the per-endpoint OUT interrupt status registers.
 *
 * @details N32H48x quirk: DAEPINTEN (0x81C) and DEEPINTEN (0x83C) only
 *          implement the OUT endpoint-0 enable bit (bit16); the OUT data
 *          endpoint enable bits (bit17..) are not writable, so DAEPINTSTS
 *          (read through @p DAINT) never reports a completed OUT transfer
 *          on EP1..EPn and GINTSTS OEPTINT never dispatches them.
 *          The per-endpoint DOUTEPINTSTS registers are still set by the
 *          core, so the data endpoints are serviced by walking those
 *          registers directly.  otg_epout_handler() clears them by
 *          write-1-to-clear, so calling this repeatedly is harmless.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
static void otg_epout_poll(USBDriver *usbp) {
  n32_otg_t   *otgp = usbp->otg;
  usbep_t     ep;

  for (ep = 1; ep <= usbp->otgparams->num_endpoints; ep++) {
    uint32_t epint = otgp->oe[ep].DOEPINT;

    if (epint == 0U)
      continue;

    /* Record a NAK before any other filtering: a NAK here means the host
       really did send an OUT to this endpoint and the core answered NAK,
       which is the decisive difference between "host never sent" and
       "host sent but the transfer descriptor was wrong". */
    if ((epint & DOEPINT_NAKIF) != 0U) {
      n32dbg_out_nak_cnt++;
      n32dbg_out_nak_ep = ep;
    }

    /* Only endpoints the core still has enabled can legitimately report a
       transfer-complete condition.*/
    if ((otgp->oe[ep].DOEPCTL & DOEPCTL_EPTENA) == 0U) {
      otgp->oe[ep].DOEPINT = 0xFFFFFFFFU;
      continue;
    }
    n32dbg_poll_cnt++;
    n32dbg_poll_ep  = ep;
    n32dbg_poll_val = epint;
    otg_epout_handler(usbp, ep);
  }
}

/**
 * @brief   Isochronous IN transfer failed handler.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
static void otg_isoc_in_failed_handler(USBDriver *usbp) {
  usbep_t ep;
  n32_otg_t *otgp = usbp->otg;

  for (ep = 0; ep <= usbp->otgparams->num_endpoints; ep++) {
    if (((otgp->ie[ep].DIEPCTL & DIEPCTL_EPTYPE_MASK) == DIEPCTL_EPTYPE_ISO) &&
        ((otgp->ie[ep].DIEPCTL & DIEPCTL_EPTENA) != 0)) {
      /* Endpoint enabled -> ISOC IN transfer failed.*/
      /* Disable endpoint.*/
      otgp->ie[ep].DIEPCTL |= (DIEPCTL_EPTDIS | DIEPCTL_SNAK);
      while (otgp->ie[ep].DIEPCTL & DIEPCTL_EPTENA)
        ;

      /* Flush FIFO.*/
      otg_txfifo_flush(usbp, ep);

      /* Prepare data for next frame.*/
      _usb_isr_invoke_in_cb(usbp, ep);
    }
  }
}

/**
 * @brief   Isochronous OUT transfer failed handler.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
static void otg_isoc_out_failed_handler(USBDriver *usbp) {
  usbep_t ep;
  n32_otg_t *otgp = usbp->otg;

  for (ep = 0; ep <= usbp->otgparams->num_endpoints; ep++) {
    if (((otgp->oe[ep].DOEPCTL & DOEPCTL_EPTYPE_MASK) == DOEPCTL_EPTYPE_ISO) &&
        ((otgp->oe[ep].DOEPCTL & DOEPCTL_EPTENA) != 0)) {
      /* Prepare transfer for next frame.*/
      _usb_isr_invoke_out_cb(usbp, ep);
    }
  }
}

/**
 * @brief   OTG shared ISR.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
/* Instrumentation for Vial debugging (defined non-static so the keyboard
   layer can read them over the UART debug port). */
volatile uint32_t n32dbg_isr_cnt;
volatile uint32_t n32dbg_rx_cnt;
volatile uint32_t n32dbg_epout_cnt;
volatile uint32_t n32dbg_rst_cnt;
volatile uint32_t n32dbg_initep_cnt;
volatile uint32_t n32dbg_initep_ep;
volatile uint32_t n32dbg_initep_outstate;
volatile uint32_t n32dbg_initep_out_enabled;
volatile uint32_t n32dbg_startout_cnt;
volatile uint32_t n32dbg_startout_ep;
volatile uint32_t n32dbg_startout_msk_before;
volatile uint32_t n32dbg_startout_msk_after;
volatile uint32_t n32dbg_usbreset_cnt;

static void usb_lld_serve_interrupt(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;
  uint32_t sts, src;
  unsigned retry = 64U;

  n32dbg_isr_cnt++;

irq_retry:

  sts  = otgp->GINTSTS;
  sts &= otgp->GINTMSK;
  /* Clear the masked-in bits we are about to service.
     NOTE: do NOT clear the whole GINTSTS here.  RXFLVL (bit4) is
     cleared as a side effect of popping the RX FIFO, and writing 1 to it
     up front destroys the very event the RX handler below needs; other
     bits (EOPF, PTXFE) are harmless stale status and are left alone.
     The earlier "ISR storm" was caused by SOF being masked off, which is
     fixed above by keeping SOF enabled. */
  otgp->GINTSTS = sts;
  n32dbg_isr_sts_or |= sts;

  /* Reset interrupt handling.  Handle both USBRSTIF (STM32-style, bit12)
     and RSTDIF (N32 "Reset detected", bit23).  On N32H48x the host bus
     reset is reported through RSTDIF; if we only listen to USBRSTIF the
     reset path never runs, DAINTMSK stays at 0x1001F (IN0..IN4 + OUT0)
     and the OUT data endpoints never get their interrupt unmasked -
     symptom: device enumerates but Vial hangs (no raw HID OUT delivery).*/
  if ((sts & GINTSTS_USBRST) || (sts & GINTSTS_RSTDIF)) {
    /* Default reset action.*/
    n32dbg_rst_cnt++;
    _usb_reset(usbp);

    /* Preventing execution of more handlers, the core has been reset.*/
    return;
  }

  /* Wake-up handling.*/
  if (sts & GINTSTS_WKUPINT) {
    /* If clocks are gated off, turn them back on (may be the case if
       coming out of suspend mode).*/
    if (otgp->PCGCCTL & PCGCCTL_STOPPCLK) {
      /* Set to zero to un-gate the USB core clocks.*/
      otgp->PCGCCTL &= ~PCGCCTL_STOPPCLK;
    }

    /* Re-enable endpoint IRQs if they have been disabled by suspend before.*/
    otg_enable_ep(usbp);

    /* Clear the Remote Wake-up Signaling.*/
    otgp->DCTL &= ~DCTL_RWKUPSIG;

    _usb_wakeup(usbp);
  }

  /* Suspend handling.*/
  if (sts & GINTSTS_USBSUSP) {
    /* Stopping all ongoing transfers.*/
    otg_disable_ep(usbp);

    /* Default suspend action.*/
    _usb_suspend(usbp);
  }

  /* Enumeration done.*/
  if (sts & GINTSTS_ENUMDONE) {
    /* Full or High speed timing selection.*/
    if ((otgp->DSTS & DSTS_ENUMSPD_MASK) == DSTS_ENUMSPD_HS_480) {
      otgp->GUSBCFG = (otgp->GUSBCFG & ~(GUSBCFG_USBTRDTIM_MASK)) |
                      GUSBCFG_USBTRDTIM(USBTRDTIM_VALUE_HS);
    }
    else {
      otgp->GUSBCFG = (otgp->GUSBCFG & ~(GUSBCFG_USBTRDTIM_MASK)) |
                      GUSBCFG_USBTRDTIM(USBTRDTIM_VALUE_FS);
    }
  }

  /* SOF interrupt handling.*/
  if (sts & GINTSTS_SOF) {
    /* Disable the SOF interrupt once it has served its purpose.
       Measured on this board: keeping SOF enabled as a "heartbeat" makes the
       ISR run at ~1 kHz and starves the main loop - the device stops
       enumerating altogether.  The endpoint polling in this ISR therefore
       must not depend on SOF; it is driven by the endpoint and RX interrupts
       that the core actually raises. */
    if (usbp->config->sof_cb == NULL) {
      otgp->GINTMSK &= ~GINTMSK_SOFMSK;
    }
    if (usbp->state == USB_SUSPENDED) {
      /* If clocks are gated off, turn them back on (may be the case if
         coming out of suspend mode).*/
      if (otgp->PCGCCTL & PCGCCTL_STOPPCLK) {
        /* Set to zero to un-gate the USB core clocks.*/
        otgp->PCGCCTL &= ~PCGCCTL_STOPPCLK;
      }

      /* Re-enable endpoint irqs if they have been disabled by suspend
         before. */
      otg_enable_ep(usbp);

      _usb_wakeup(usbp);
    }

    _usb_isr_invoke_sof_cb(usbp);
  }

  /* Isochronous IN failed handling */
  if (sts & GINTSTS_INCOMPISOIN) {
    otg_isoc_in_failed_handler(usbp);
  }

  /* Isochronous OUT failed handling */
  if (sts & GINTSTS_INCOMPISOOUT) {
    otg_isoc_out_failed_handler(usbp);
  }

  /* IN/OUT endpoints event handling.*/
  src = otgp->DAINT;
  if (sts & GINTSTS_OEPTINT) {
    n32dbg_epout_cnt++;
    if (src & (1 << 16))
      otg_epout_handler(usbp, 0);
    if (src & (1 << 17))
      otg_epout_handler(usbp, 1);
    if (src & (1 << 18))
      otg_epout_handler(usbp, 2);
    if (src & (1 << 19))
      otg_epout_handler(usbp, 3);
#if USB_MAX_ENDPOINTS >= 4
    if (src & (1 << 20))
      otg_epout_handler(usbp, 4);
#endif
#if USB_MAX_ENDPOINTS >= 5
    if (src & (1 << 21))
      otg_epout_handler(usbp, 5);
#endif
#if USB_MAX_ENDPOINTS >= 6
    if (src & (1 << 22))
      otg_epout_handler(usbp, 6);
#endif
#if USB_MAX_ENDPOINTS >= 7
    if (src & (1 << 23))
      otg_epout_handler(usbp, 7);
#endif
#if USB_MAX_ENDPOINTS >= 8
    if (src & (1 << 24))
      otg_epout_handler(usbp, 8);
#endif
    /* OUT endpoints 9..15 are reported in DAEPINT (USBHS_DAEPINTSTS) bits
       25..31 - the only OUT data endpoints this device actually implements
       besides EP0.  Reference manual, USBHS_DAEPINTEN: OUTEPIEN[15:9] occupy
       bits 31:25, while bits 24:17 are reserved (there is no OUT EP1..EP8). */
#if USB_MAX_ENDPOINTS >= 9
    if (src & (1 << 25))
      otg_epout_handler(usbp, 9);
#endif
#if USB_MAX_ENDPOINTS >= 10
    if (src & (1 << 26))
      otg_epout_handler(usbp, 10);
#endif
#if USB_MAX_ENDPOINTS >= 11
    if (src & (1 << 27))
      otg_epout_handler(usbp, 11);
#endif
#if USB_MAX_ENDPOINTS >= 12
    if (src & (1 << 28))
      otg_epout_handler(usbp, 12);
#endif
#if USB_MAX_ENDPOINTS >= 13
    if (src & (1 << 29))
      otg_epout_handler(usbp, 13);
#endif
#if USB_MAX_ENDPOINTS >= 14
    if (src & (1 << 30))
      otg_epout_handler(usbp, 14);
#endif
#if USB_MAX_ENDPOINTS >= 15
    if (src & (1U << 31))
      otg_epout_handler(usbp, 15);
#endif
  }
  if (sts & GINTSTS_IEPTINT) {
    if (src & (1 << 0))
      otg_epin_handler(usbp, 0);
    if (src & (1 << 1))
      otg_epin_handler(usbp, 1);
    if (src & (1 << 2))
      otg_epin_handler(usbp, 2);
    if (src & (1 << 3))
      otg_epin_handler(usbp, 3);
#if USB_MAX_ENDPOINTS >= 4
    if (src & (1 << 4))
      otg_epin_handler(usbp, 4);
#endif
#if USB_MAX_ENDPOINTS >= 5
    if (src & (1 << 5))
      otg_epin_handler(usbp, 5);
#endif
#if USB_MAX_ENDPOINTS >= 6
    if (src & (1 << 6))
      otg_epin_handler(usbp, 6);
#endif
#if USB_MAX_ENDPOINTS >= 7
    if (src & (1 << 7))
      otg_epin_handler(usbp, 7);
#endif
#if USB_MAX_ENDPOINTS >= 8
    if (src & (1 << 8))
      otg_epin_handler(usbp, 8);
#endif
  }

  /* Performing the whole FIFO emptying in the STS, it is advised to keep
     this IRQ at a very low priority level.*/
  if ((sts & GINTSTS_RXFLVL) != 0U) {
    n32dbg_rx_cnt++;
    otg_rxfifo_handler(usbp);
  }

  /* N32H48x: the OUT data endpoints can never be unmasked through
     DAEPINTEN (only the EP0 bit is implemented), so the DAINT bitmap above
     only ever dispatches EP0.  Service EP1..EPn from their own interrupt
     status registers on every pass of this handler.*/
  otg_epout_poll(usbp);

  /* Same limitation on the IN side: completed IN transfers (TXFE/NAKIF on
     DIEPINTn) are not reflected in DAINT either, so a reply queued with
     send_report() would sit in the endpoint forever while the host waits.
     Walk DIEPINTn directly, mirroring otg_epout_poll().  EP0 is driven by
     the normal control path, so start at EP1.  Skip endpoints that have no
     driver config to keep this cheap and safe. */
  {
    usbep_t ep;
    for (ep = 1; ep <= usbp->otgparams->num_endpoints; ep++) {
      uint32_t epint;

      if ((usbp->epc[ep] == NULL) || (usbp->epc[ep]->in_state == NULL))
        continue;

      epint = otgp->ie[ep].DIEPINT;
      if (epint == 0U)
        continue;

      /* Ignore endpoints the core has disabled (unused). */
      if ((otgp->ie[ep].DIEPCTL & DIEPCTL_EPTENA) == 0U &&
          (otgp->ie[ep].DIEPCTL & DIEPCTL_USBACEPT) == 0U) {
        otgp->ie[ep].DIEPINT = 0xFFFFFFFFU;
        continue;
      }

      n32dbg_epin_poll_cnt++;
      n32dbg_epin_poll_ep  = ep;
      n32dbg_epin_poll_val = epint;
      otg_epin_handler(usbp, ep);
    }
  }

  if ((sts & IRQ_RETRY_MASK) && (--retry > 0U))
    goto irq_retry;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if N32_USB_USE_OTG1 || defined(__DOXYGEN__)
/**
 * @brief   USBHS global interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USB_HS_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  usb_lld_serve_interrupt(&USBD1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   USBHS EP IN global interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USB_HS_EPx_IN_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  usb_lld_serve_interrupt(&USBD1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   USBHS EP OUT global interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USB_HS_EPx_OUT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  usb_lld_serve_interrupt(&USBD1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   USBHS wakeup interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USB_HS_WKUP_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  usb_lld_serve_interrupt(&USBD1);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level USB driver initialization.
 *
 * @notapi
 */
void usb_lld_init(void) {

  /* Driver initialization.*/
#if N32_USB_USE_OTG1
  usbObjectInit(&USBD1);
  USBD1.otg       = N32_USBHS_BASE;
  USBD1.otgparams = &hsparams;
#endif
}

/**
 * @brief   Configures and activates the USB peripheral.
 * @note    Starting the OTG cell can be a slow operation carried out with
 *          interrupts disabled, perform it before starting time-critical
 *          operations.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_start(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;

  if (usbp->state == USB_STOP) {
    /* Clock activation.*/

#if N32_USB_USE_OTG1
    if (&USBD1 == usbp) {
      /* USBHS clock enable and reset.*/
      usb_clock_enable();
      usb_clock_reset();

      /* Enables IRQ vector.*/
      nvicEnableVector(USB_HS_IRQn, N32_USB_OTG1_IRQ_PRIORITY);
      nvicEnableVector(USB_HS_EPx_OUT_IRQn, N32_USB_OTG1_IRQ_PRIORITY);
      nvicEnableVector(USB_HS_EPx_IN_IRQn, N32_USB_OTG1_IRQ_PRIORITY);
      nvicEnableVector(USB_HS_WKUP_IRQn, N32_USB_OTG1_IRQ_PRIORITY);
    }
#endif

    /* PHY enabled.*/
    otgp->PCGCCTL = 0;

    /* Soft core reset.  Must be done BEFORE programming GUSBCFG/DCFG:
       the reset returns those registers to their default values, so a
       device-mode write issued before the reset would be lost. */
    otg_core_reset(usbp);

#if N32_USB_USE_OTG1
    if (&USBD1 == usbp) {
      /* - Forced device mode.
         - USB turn-around time = USBTRDTIM_VALUE_FS.
         - Built-in Full Speed PHY: the SDK clears GCFG.PHYSEL in
           USB_CoreInit() (PHYSEL=1 would select an external PHY and
           leave DP/DM undriven -- the host would never see the device),
           so GUSBCFG must NOT set PHYSEL here. */
      otgp->GUSBCFG = GUSBCFG_FDEVMODE | GUSBCFG_USBTRDTIM(USBTRDTIM_VALUE_FS);

      /* Device speed: HS PHY running in Full Speed (DEVSPD=1), matching
         SDK USB_InitDevSpeed(USB_SPEED_PARAM_HIGH_IN_FULL). The previous
         0x02200000 value was an AT32-import leftover that wrote into the
         PSITVL/reserved bits. */
      otgp->DCFG = DCFG_DEVSPD_HS_FS;
    }
#endif

    /* Interrupts on TXFIFOs half empty.*/
    otgp->GAHBCFG = 0;

    /* Endpoints re-initialization.*/
    otg_disable_ep(usbp);

    /* Clear all pending Device Interrupts, only the USB Reset interrupt
       is required initially.*/
    otgp->DIEPMSK  = 0;
    otgp->DOEPMSK  = 0;
    otgp->DAINTMSK = 0;
    otgp->GINTMSK  = GINTMSK_ENUMDONEMSK | GINTMSK_USBRSTMSK |
                     GINTMSK_USBSUSPMSK | GINTMSK_ERLYSUSPMSK |
                     GINTMSK_WKUPINTMSK | GINTMSK_INCOMISOINMSK |
                     GINTMSK_INCOMPISOOUTMSK |
                     GINTMSK_RSTDIFMSK;

    /* Clears all pending IRQs, if any. */
    otgp->GINTSTS  = 0xFFFFFFFF;

    /* Global interrupts enable.*/
    otgp->GAHBCFG |= GAHBCFG_GLBINTMSK;
  }
}

/**
 * @brief   Deactivates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_stop(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;

  /* If in ready state then disables the USB clock.*/
  if (usbp->state != USB_STOP) {

    /* Disabling all endpoints in case the driver has been stopped while
       active.*/
    otg_disable_ep(usbp);

    otgp->DAINTMSK   = 0;
    otgp->GAHBCFG    = 0;

#if N32_USB_USE_OTG1
    if (&USBD1 == usbp) {
      nvicDisableVector(USB_HS_IRQn);
      nvicDisableVector(USB_HS_EPx_OUT_IRQn);
      nvicDisableVector(USB_HS_EPx_IN_IRQn);
      nvicDisableVector(USB_HS_WKUP_IRQn);
      usb_clock_disable();
    }
#endif
  }
}

/**
 * @brief   USB low level reset routine.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_reset(USBDriver *usbp) {
  unsigned i;
  n32_otg_t *otgp = usbp->otg;

  /* Flush the Tx FIFO.*/
  otg_txfifo_flush(usbp, 0);

  /* Endpoint interrupts all disabled and cleared.*/
  otgp->DIEPEMPMSK = 0;
  otgp->DAINTMSK   = DAINTMSK_OUTEPTMSK(0) | DAINTMSK_INEPTMSK(0);

  /* All endpoints in NAK mode, interrupts cleared.*/
  for (i = 0; i <= usbp->otgparams->num_endpoints; i++) {
    otgp->ie[i].DIEPCTL = DIEPCTL_SNAK;
    otgp->oe[i].DOEPCTL = DOEPCTL_SNAK;
    otgp->ie[i].DIEPINT = 0xFFFFFFFF;
    otgp->oe[i].DOEPINT = 0xFFFFFFFF;
  }

  /* Resets the FIFO memory allocator.*/
  otg_ram_reset(usbp);

  /* Receive FIFO size initialization, the address is always zero.*/
  otgp->GRXFSIZ = usbp->otgparams->rx_fifo_size;
  otg_rxfifo_flush(usbp);

  /* Resets the device address to zero.*/
  otgp->DCFG = (otgp->DCFG & ~DCFG_DEVADDR_MASK) | DCFG_DEVADDR(0);

  /* Enables also EP-related interrupt sources.*/
  otgp->GINTMSK  |= GINTMSK_RXFLVLMSK  | GINTMSK_OEPTINTMSK |
                    GINTMSK_IEPTINTMSK;
  otgp->DIEPMSK   = DIEPMSK_TIMEOUTMSK | DIEPMSK_XFERCMSK;
  /* NOTE: do NOT enable NAK/packet-error interrupts here.  NAKIF is asserted
     continuously while an armed OUT endpoint has no free packet, and for EP0
     during every control transfer; enabling it turns the USB ISR into an
     interrupt storm that starves the main loop (the UART debug dump and
     matrix scan stop running entirely).  NAK is instead sampled
     non-intrusively from thread context in n32dbg_usb_probe(). */
  otgp->DOEPMSK   = DOEPMSK_SETUPMSK   | DOEPMSK_XFERCMSK;

  /* EP0 initialization, it is a special case.*/
  usbp->epc[0] = &ep0config;
  otgp->oe[0].DOEPTSIZ = DOEPTSIZ_SETUPCNT(3);
  otgp->oe[0].DOEPCTL = DOEPCTL_SETD0PID | DOEPCTL_USBACEPT |
                        DOEPCTL_EPTYPE_CTRL | DOEPCTL_MPS(ep0config.out_maxsize);
  otgp->ie[0].DIEPTSIZ = 0;
  otgp->ie[0].DIEPCTL = DIEPCTL_SETD0PID | DIEPCTL_USBACEPT |
                        DIEPCTL_EPTYPE_CTRL | DIEPCTL_TXFNUM(0) |
                        DIEPCTL_MPS(ep0config.in_maxsize);
  otgp->DIEPTXF0 = DIEPTXF_INEPTXFDEP(ep0config.in_maxsize / 4) |
                   DIEPTXF_INEPTXFSTADDR(otg_ram_alloc(usbp,
                                                  ep0config.in_maxsize / 4));

  /* Re-arm the interrupts of the endpoints that are already registered in
     the endpoint table. A bus reset can happen after the host has configured
     the device (re-enumeration, cable glitch, host resume); in that case the
     host may not send another SET_CONFIGURATION, so the data endpoint
     interrupts would remain masked forever - the device keeps enumerating but
     never delivers OUT reports (symptom: enumeration OK, Vial hangs).
     Re-arming from the registered configuration is safe: the endpoints are
     not enabled in hardware yet (they have just been put in NAK mode above),
     so no spurious interrupts are generated. */
  for (i = 1U; i <= usbp->otgparams->num_endpoints; i++) {
    /* Arm the data-endpoint interrupt bits.  The endpoint table is populated
       by the HID stack only after SET_CONFIGURATION; a reset can (re)occur
       before that, so relying on the registered in_state/out_state pointers
       here races with the (re-)enumeration.  Unused endpoints sit in NAK mode
       and cannot raise spurious interrupts, so arming every *implemented* bit
       on every reset is safe.

       Which bits exist is fixed by the hardware (reference manual,
       USBHS_DAEPINTEN): INEPIEN[8:0] for IN endpoints 0..8, OUTEPIEN0 for
       OUT0, and OUTEPIEN[15:9] for OUT 9..15.  Bits 24:17 (OUT EP1..EP8) and
       bits 15:9 are reserved, so they must never be set. */
    if (i <= 8U) {
      otgp->DAINTMSK |= DAINTMSK_INEPTMSK(i);
    }
    if ((i == 0U) || (i >= 9U)) {
      otgp->DAINTMSK |= DAINTMSK_OUTEPTMSK(i);
    }
  }
}

/**
 * @brief   Sets the USB address.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_set_address(USBDriver *usbp) {
  n32_otg_t *otgp = usbp->otg;

  n32dbg_setaddr_cnt++;
  otgp->DCFG = (otgp->DCFG & ~DCFG_DEVADDR_MASK) | DCFG_DEVADDR(usbp->address);
}

/**
 * @brief   Enables an endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_init_endpoint(USBDriver *usbp, usbep_t ep) {
  uint32_t ctl, fsize;
  n32_otg_t *otgp = usbp->otg;

  /* IN and OUT common parameters.*/
  switch (usbp->epc[ep]->ep_mode & USB_EP_MODE_TYPE) {
  case USB_EP_MODE_TYPE_CTRL:
    ctl = DIEPCTL_SETD0PID | DIEPCTL_USBACEPT | DIEPCTL_EPTYPE_CTRL;
    break;
  case USB_EP_MODE_TYPE_ISOC:
    ctl = DIEPCTL_SETD0PID | DIEPCTL_USBACEPT | DIEPCTL_EPTYPE_ISO;
    break;
  case USB_EP_MODE_TYPE_BULK:
    ctl = DIEPCTL_SETD0PID | DIEPCTL_USBACEPT | DIEPCTL_EPTYPE_BULK;
    break;
  case USB_EP_MODE_TYPE_INTR:
    ctl = DIEPCTL_SETD0PID | DIEPCTL_USBACEPT | DIEPCTL_EPTYPE_INTR;
    break;
  default:
    return;
  }

  /* OUT endpoint activation or deactivation.*/
  n32dbg_initep_cnt++;
  n32dbg_initep_ep = ep;
  n32dbg_initep_outstate = (uint32_t)usbp->epc[ep]->out_state;
  otgp->oe[ep].DOEPTSIZ = 0;
  if (usbp->epc[ep]->out_state != NULL) {
    n32dbg_initep_out_enabled++;
    n32dbg_initep_out_mask |= (1U << ep);
    otgp->oe[ep].DOEPCTL = ctl | DOEPCTL_MPS(usbp->epc[ep]->out_maxsize);
    otgp->DAINTMSK |= DAINTMSK_OUTEPTMSK(ep);
  }
  else {
    otgp->oe[ep].DOEPCTL &= ~DOEPCTL_USBACEPT;
    /* Do NOT clear the DAINTMSK OUT bit here.  The bus-reset re-arm loop
       arms every OUT/IN data-endpoint interrupt bit unconditionally; if we
       cleared it for an endpoint that the host has not configured yet, a
       reset that happens before SET_CONFIGURATION would leave the OUT
       interrupt masked for good (symptom: RAW HID OUT reports never
       delivered, Vial hangs).  end-points stay in NAK, so no spurious
       interrupts can occur. */
  }

  /* IN endpoint activation or deactivation.*/
  otgp->ie[ep].DIEPTSIZ = 0;
  if (usbp->epc[ep]->in_state != NULL) {
    /* FIFO allocation for the IN endpoint.*/
    fsize = usbp->epc[ep]->in_maxsize / 4;
    if (usbp->epc[ep]->in_multiplier > 1)
      fsize *= usbp->epc[ep]->in_multiplier;
    otgp->DIEPTXF[ep - 1] = DIEPTXF_INEPTXFDEP(fsize) |
                            DIEPTXF_INEPTXFSTADDR(otg_ram_alloc(usbp, fsize));
    otg_txfifo_flush(usbp, ep);

    otgp->ie[ep].DIEPCTL = ctl |
                           DIEPCTL_TXFNUM(ep) |
                           DIEPCTL_MPS(usbp->epc[ep]->in_maxsize);
    otgp->DAINTMSK |= DAINTMSK_INEPTMSK(ep);
  }
  else {
    otgp->DIEPTXF[ep - 1] = 0x02000400; /* Reset value.*/
    otg_txfifo_flush(usbp, ep);
    otgp->ie[ep].DIEPCTL &= ~DIEPCTL_USBACEPT;
    /* See the OUT branch above: do not clear the DAINTMSK IN bit either. */
  }
}

/**
 * @brief   Disables all the active endpoints except the endpoint zero.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_disable_endpoints(USBDriver *usbp) {

  /* Resets the FIFO memory allocator.*/
  otg_ram_reset(usbp);

  /* Disabling all endpoints.*/
  otg_disable_ep(usbp);
}

/**
 * @brief   Returns the status of an OUT endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_out(USBDriver *usbp, usbep_t ep) {
  uint32_t ctl;

  (void)usbp;

  ctl = usbp->otg->oe[ep].DOEPCTL;
  if (!(ctl & DOEPCTL_USBACEPT))
    return EP_STATUS_DISABLED;
  if (ctl & DOEPCTL_STALL)
    return EP_STATUS_STALLED;
  return EP_STATUS_ACTIVE;
}

/**
 * @brief   Returns the status of an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_in(USBDriver *usbp, usbep_t ep) {
  uint32_t ctl;

  (void)usbp;

  ctl = usbp->otg->ie[ep].DIEPCTL;
  if (!(ctl & DIEPCTL_USBACEPT))
    return EP_STATUS_DISABLED;
  if (ctl & DIEPCTL_STALL)
    return EP_STATUS_STALLED;
  return EP_STATUS_ACTIVE;
}

/**
 * @brief   Reads a setup packet from the dedicated packet buffer.
 * @details This function must be invoked in the context of the @p setup_cb
 *          callback in order to read the received setup packet.
 * @pre     In order to use this function the endpoint must have been
 *          initialized as a control endpoint.
 * @post    The endpoint is ready to accept another packet.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @param[out] buf      buffer where to copy the packet data
 *
 * @notapi
 */
void usb_lld_read_setup(USBDriver *usbp, usbep_t ep, uint8_t *buf) {

  memcpy(buf, usbp->epc[ep]->setup_buf, 8);
}

/**
 * @brief   Starts a receive operation on an OUT endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_out(USBDriver *usbp, usbep_t ep) {
  uint32_t pcnt, rxsize, ctl;
  unsigned guard;
  USBOutEndpointState *osp = usbp->epc[ep]->out_state;

  /* Transfer initialization.*/
  osp->totsize = osp->rxsize;
  if ((ep == 0) && (osp->rxsize > EP0_MAX_OUTSIZE))
    osp->rxsize = EP0_MAX_OUTSIZE;

  /* Transaction size is rounded to a multiple of packet size because the
     following requirement in the RM:
     "For OUT transfers, the transfer size field in the endpoint's transfer
     size register must be a multiple of the maximum packet size of the
     endpoint, adjusted to the Word boundary".*/
  pcnt   = (osp->rxsize + usbp->epc[ep]->out_maxsize - 1U) /
           usbp->epc[ep]->out_maxsize;
  /* N32-specific: unlike STM32/AT32 OTG cores, the N32 OTG core does NOT
     attach any special meaning to PKTCNT == 0 on EP0 (STM32 treats it as
     "wait for SETUP / accept ZLP status OUT").  If the transfer size is
     zero (EP0 status stage -- zero length OUT packet, which every control
     transfer ends with) the core never finishes the transaction and the
     host keeps re-sending the same request.  The vendor SDK programs
     TLEN = maxpacket, PKTCNT = 1 for zero-length OUT, so mirror that:
     force pcnt >= 1 and a word-aligned size of at least one packet. */
  if (pcnt == 0U) {
    pcnt   = 1U;
    rxsize = usbp->epc[ep]->out_maxsize;
  }
  else {
    rxsize = (pcnt * usbp->epc[ep]->out_maxsize + 3U) & 0xFFFFFFFCU;
  }

  /* DWC rule: DOEPTSIZ may only be written while the endpoint is disabled.
     If EPENA is still set (the endpoint was left armed by an earlier
     transfer that never completed -- e.g. EP3 armed at SET_CONFIGURATION
     and the host never sent a report), writing the size register is silently
     ignored by the core and the endpoint keeps waiting on the OLD
     descriptor.  Symptom: the host issues the OUT packet, the core never
     accepts it, WriteFile() blocks forever, and neither DOEPINT nor the RX
     status queue ever shows the packet.  Disable first, wait for the
     completion of the disable (EPDISD), then re-program. */
  ctl = usbp->otg->oe[ep].DOEPCTL;
  if ((ctl & DOEPCTL_EPTENA) != 0U) {
    n32dbg_out_epena_busy++;
    usbp->otg->oe[ep].DOEPCTL = (ctl | DOEPCTL_EPTDIS) & ~DOEPCTL_EPTENA;
    for (guard = 0; guard < 2000U; guard++) {
      if ((usbp->otg->oe[ep].DOEPCTL & DOEPCTL_EPTENA) == 0U)
        break;
      n32dbg_out_epdis_wait++;
    }
    if (guard >= 2000U)
      n32dbg_out_epdis_tmo++;
  }

  /* Setting up transaction parameters in DOEPTSIZ.
     The SETUPCNT field must be programmed for EVERY OUT endpoint, not only
     EP0.  Both reference DWC implementations do exactly this:
       ChibiOS STM32 OTGv1  DOEPTSIZ_STUPCNT(3) | PKTCNT | XFRSIZ
       ChibiOS-Contrib AT32 DOEPTSIZ_SETUPCNT(3) | PKTCNT | XFERSIZE
     Programming 0 here for a data endpoint left it permanently unable to
     accept a host OUT packet: PKTCNT never decremented, DOEPINT stayed 0 and
     no RX status word was ever produced, even though the OUT token did reach
     the core (a STALLed endpoint rejected the host write immediately).  The
     earlier "only EP0 needs SETUPCNT" note was wrong and caused the raw HID
     OUT endpoint to never receive anything. */
  usbp->otg->oe[ep].DOEPTSIZ = DOEPTSIZ_SETUPCNT(3) |
                               DOEPTSIZ_PKTCNT(pcnt) |
                               DOEPTSIZ_XFERSIZE(rxsize);

  /* Special case of isochronous endpoint.*/
  if ((usbp->epc[ep]->ep_mode & USB_EP_MODE_TYPE) == USB_EP_MODE_TYPE_ISOC) {
    /* Odd/even bit toggling for isochronous endpoint.*/
    if (usbp->otg->DSTS & DSTS_SOFFN_ODD)
      usbp->otg->oe[ep].DOEPCTL |= DOEPCTL_SETEVENFR;
    else
      usbp->otg->oe[ep].DOEPCTL |= DOEPCTL_SETODDFR;
  }

  /* Make sure the endpoint interrupt is unmasked before arming the endpoint.
     On this part a bus reset (or the disable-all path) clears DAINTMSK down to
     OUT0|IN0 and the OUT interrupts of the data endpoints are only restored
     when the host re-sends SET_CONFIGURATION. Re-arming here guarantees the
     completion interrupt of the transfer we are about to start is delivered,
     even if the enumeration path did not re-enable it. */
  n32dbg_startout_cnt++;
  n32dbg_startout_ep = ep;
  n32dbg_startout_mask |= (1U << ep);
  n32dbg_startout_msk_before = usbp->otg->DAINTMSK;
  usbp->otg->DAINTMSK |= DAINTMSK_OUTEPTMSK(ep);
  n32dbg_startout_msk_after = usbp->otg->DAINTMSK;

  /* Clear any stale status bits for this endpoint before arming, otherwise a
     leftover XFERC/NAK bit makes the completion handler run on a transfer that
     never happened. */
  usbp->otg->oe[ep].DOEPINT = 0xFFFFFFFFU;

  /* Starting operation: one single write of CNAK|EPENA (the vendor SDK does
     the same) so the core cannot sample a half-updated control register. */
  usbp->otg->oe[ep].DOEPCTL |= DOEPCTL_CNAK | DOEPCTL_EPTENA;

  n32dbg_out_arm_ep  = ep;
  n32dbg_out_arm_ctl = usbp->otg->oe[ep].DOEPCTL;
  n32dbg_out_arm_siz = usbp->otg->oe[ep].DOEPTSIZ;
}

/**
 * @brief   Starts a transmit operation on an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_in(USBDriver *usbp, usbep_t ep) {
  USBInEndpointState *isp = usbp->epc[ep]->in_state;

  n32dbg_startin_cnt++;
  n32dbg_startin_ep  = ep;
  n32dbg_startin_sz  = isp->txsize;

  /* Transfer initialization.*/
  isp->totsize = isp->txsize;
  if (isp->txsize == 0) {
    /* Special case, sending zero size packet.*/
    usbp->otg->ie[ep].DIEPTSIZ = DIEPTSIZ_PKTCNT(1) | DIEPTSIZ_XFERSIZE(0);
  }
  else {
    if ((ep == 0) && (isp->txsize > EP0_MAX_INSIZE))
      isp->txsize = EP0_MAX_INSIZE;

    /* Normal case.*/
    uint32_t pcnt = (isp->txsize + usbp->epc[ep]->in_maxsize - 1) /
                    usbp->epc[ep]->in_maxsize;
    /* CHTODO: Support more than one packet per frame for isochronous
       transfers.*/
    usbp->otg->ie[ep].DIEPTSIZ = DIEPTSIZ_MC(1) | DIEPTSIZ_PKTCNT(pcnt) |
                                 DIEPTSIZ_XFERSIZE(isp->txsize);
  }

  /* Special case of isochronous endpoint.*/
  if ((usbp->epc[ep]->ep_mode & USB_EP_MODE_TYPE) == USB_EP_MODE_TYPE_ISOC) {
    /* Odd/even bit toggling.*/
    if (usbp->otg->DSTS & DSTS_SOFFN_ODD)
      usbp->otg->ie[ep].DIEPCTL |= DIEPCTL_SETEVENFR;
    else
      usbp->otg->ie[ep].DIEPCTL |= DIEPCTL_SETODDFR;
  }

  /* Starting operation.*/
  usbp->otg->ie[ep].DIEPCTL |= DIEPCTL_EPTENA | DIEPCTL_CNAK;
  usbp->otg->DIEPEMPMSK |= DIEPEMPMSK_INEPTXFEMSK(ep);
}

/**
 * @brief   Brings an OUT endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_out(USBDriver *usbp, usbep_t ep) {

  n32dbg_stall_out_cnt++;
  n32dbg_stall_out_ep = ep;
  usbp->otg->oe[ep].DOEPCTL |= DOEPCTL_STALL;
}

/**
 * @brief   Brings an IN endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_in(USBDriver *usbp, usbep_t ep) {

  n32dbg_stall_in_cnt++;
  n32dbg_stall_in_ep = ep;
  usbp->otg->ie[ep].DIEPCTL |= DIEPCTL_STALL;
}

/**
 * @brief   Brings an OUT endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_out(USBDriver *usbp, usbep_t ep) {

  usbp->otg->oe[ep].DOEPCTL &= ~DOEPCTL_STALL;
}

/**
 * @brief   Brings an IN endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_in(USBDriver *usbp, usbep_t ep) {

  usbp->otg->ie[ep].DIEPCTL &= ~DIEPCTL_STALL;
}

#endif /* HAL_USE_USB */

/** @} */
