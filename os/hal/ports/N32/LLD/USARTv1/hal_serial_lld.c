/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio
    ChibiOS - Copyright (C) 2026 bvbhu (N32H48x port for vial-qmk-stg)

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
 * @file    N32/LLD/USARTv1/hal_serial_lld.c
 * @brief   N32H48x low level serial driver code.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "hal.h"

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   Maximum frames serviced per RX interrupt.
 * @details The loop is bounded so that a flag which cannot be cleared (for
 *          example because the optional FIFO is enabled by other code) can
 *          never lock the CPU inside an ISR.
 */
#define N32_SERIAL_RX_GUARD                 16U

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USART1 serial driver identifier.*/
#if N32_SERIAL_USE_USART1
SerialDriver SD1;
#endif
/** @brief USART2 serial driver identifier.*/
#if N32_SERIAL_USE_USART2
SerialDriver SD2;
#endif
/** @brief USART3 serial driver identifier.*/
#if N32_SERIAL_USE_USART3
SerialDriver SD3;
#endif
/** @brief USART4 serial driver identifier.*/
#if N32_SERIAL_USE_USART4
SerialDriver SD4;
#endif
/** @brief UART5 serial driver identifier.*/
#if N32_SERIAL_USE_UART5
SerialDriver SD5;
#endif
/** @brief UART6 serial driver identifier.*/
#if N32_SERIAL_USE_UART6
SerialDriver SD6;
#endif
/** @brief UART7 serial driver identifier.*/
#if N32_SERIAL_USE_UART7
SerialDriver SD7;
#endif
/** @brief UART8 serial driver identifier.*/
#if N32_SERIAL_USE_UART8
SerialDriver SD8;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/** @brief Driver default configuration: 38400 8N1, one stop bit.*/
static const SerialConfig default_config = {
  SERIAL_DEFAULT_BITRATE,
  0U,
  N32_USART_CTRL2_STOP1_BITS,
  0U
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   USART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration
 */
static void usart_init(SerialDriver *sdp, const SerialConfig *config) {
  USART_Module *u = sdp->usart;
  uint32_t brcf;

  osalDbgAssert(config->speed != 0U, "invalid bit rate");

  /* BRCF is a 12.4 fixed-point USARTDIV: with 16x oversampling the value
     is simply fCK / baud.  The N32H48x UARTs also offer 8x oversampling
     (CTRL1.OSPM); in that mode the mantissa doubles and only the lower
     three fractional bits are used, exactly like STM32 OVER8. */
  brcf = (sdp->clock + (config->speed / 2U)) / config->speed;
  if ((config->ctrl1 & USART_CTRL1_OSPM) != 0U) {
    brcf = ((brcf & ~7U) * 2U) | (brcf & 7U);
    brcf &= ~8U;                    /* bit 3 reserved in 8x mode */
  }
  osalDbgAssert((brcf >= 16U) && (brcf < 0x10000U), "invalid BRCF value");

  /* Disable while configuring and keep the optional FIFO off: this driver
     uses the legacy single-register STS/DAT behaviour shared with the
     STM32 USARTv1 driver. */
  u->CTRL1 = 0U;
  u->FIFO  = 0U;
  u->BRCF  = brcf;

  /* Some bits are enforced by the driver.  TXDEIE/TCIE are deliberately not
     set here, they are turned on by the output-queue notify callback. */
  u->CTRL2 = config->ctrl2;
  u->CTRL3 = config->ctrl3 | USART_CTRL3_ERRIEN;
  u->CTRL1 = config->ctrl1 | USART_CTRL1_UEN | USART_CTRL1_PEIEN |
                             USART_CTRL1_RXDNEIEN | USART_CTRL1_RXEN |
                             USART_CTRL1_TXEN;

  /* Clear stale flags: write the status register, then read status and data
     (the STM32-compatible "reset step 1 / step 2" sequence). */
  u->STS = 0U;
  (void)u->STS;
  (void)u->DAT;

  /* Deciding mask to be applied on the data register on receive, this is
     required in order to mask out the parity bit. */
  if ((config->ctrl1 & (USART_CTRL1_WL | USART_CTRL1_PCEN)) ==
      USART_CTRL1_PCEN) {
    sdp->rxmask = 0x7F;
  }
  else {
    sdp->rxmask = 0xFF;
  }
}

/**
 * @brief   USART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] u         pointer to an USART I/O block
 */
static void usart_deinit(USART_Module *u) {

  u->CTRL1 = 0U;
  u->CTRL2 = 0U;
  u->CTRL3 = 0U;
  u->FIFO  = 0U;
}

/**
 * @brief   Enables and resets the peripheral clock of an USART instance.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 */
static void usart_clock_enable(SerialDriver *sdp) {

  *sdp->rst_reg |= sdp->rst_bit;
  *sdp->rst_reg &= ~sdp->rst_bit;
  *sdp->clk_reg |= sdp->clk_bit;
}

/**
 * @brief   Disables the peripheral clock of an USART instance.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 */
static void usart_clock_disable(SerialDriver *sdp) {

  *sdp->clk_reg &= ~sdp->clk_bit;
}

/**
 * @brief   Error handling routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] sts       USART STS register value
 */
static void set_error(SerialDriver *sdp, uint32_t sts) {
  eventflags_t status = 0;

  if ((sts & USART_STS_OREF) != 0U)
    status |= SD_OVERRUN_ERROR;
  if ((sts & USART_STS_PEF) != 0U)
    status |= SD_PARITY_ERROR;
  if ((sts & USART_STS_FEF) != 0U)
    status |= SD_FRAMING_ERROR;
  if ((sts & USART_STS_NEF) != 0U)
    status |= SD_NOISE_ERROR;
  chnAddFlagsI(sdp, status);
}

/**
 * @brief   Output queue notification callbacks.
 * @details Each callback arms the TX-empty and TX-complete interrupts of its
 *          own USART.  A macro is used because the eight instances differ
 *          only by the register block.
 */
#define N32_SERIAL_NOTIFY(fn, dev)                                          \
  static void fn(io_queue_t *qp) {                                          \
    (void)qp;                                                               \
    (dev)->CTRL1 |= USART_CTRL1_TXDEIEN | USART_CTRL1_TXCIEN;               \
  }

#if N32_SERIAL_USE_USART1
N32_SERIAL_NOTIFY(notify1, USART1)
#endif
#if N32_SERIAL_USE_USART2
N32_SERIAL_NOTIFY(notify2, USART2)
#endif
#if N32_SERIAL_USE_USART3
N32_SERIAL_NOTIFY(notify3, USART3)
#endif
#if N32_SERIAL_USE_USART4
N32_SERIAL_NOTIFY(notify4, USART4)
#endif
#if N32_SERIAL_USE_UART5
N32_SERIAL_NOTIFY(notify5, UART5)
#endif
#if N32_SERIAL_USE_UART6
N32_SERIAL_NOTIFY(notify6, UART6)
#endif
#if N32_SERIAL_USE_UART7
N32_SERIAL_NOTIFY(notify7, UART7)
#endif
#if N32_SERIAL_USE_UART8
N32_SERIAL_NOTIFY(notify8, UART8)
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if N32_SERIAL_USE_USART1
#if defined(N32_USART1_SUPPRESS_ISR)
#error "N32_USART1_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   USART1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USART1_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD1);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if N32_SERIAL_USE_USART2
#if defined(N32_USART2_SUPPRESS_ISR)
#error "N32_USART2_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   USART2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USART2_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD2);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if N32_SERIAL_USE_USART3
#if defined(N32_USART3_SUPPRESS_ISR)
#error "N32_USART3_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   USART3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USART3_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD3);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if N32_SERIAL_USE_USART4
#if defined(N32_USART4_SUPPRESS_ISR)
#error "N32_USART4_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   USART4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_USART4_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD4);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if N32_SERIAL_USE_UART5
#if defined(N32_UART5_SUPPRESS_ISR)
#error "N32_UART5_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   UART5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_UART5_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD5);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if N32_SERIAL_USE_UART6
#if defined(N32_UART6_SUPPRESS_ISR)
#error "N32_UART6_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   UART6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_UART6_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD6);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if N32_SERIAL_USE_UART7
#if defined(N32_UART7_SUPPRESS_ISR)
#error "N32_UART7_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   UART7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_UART7_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD7);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if N32_SERIAL_USE_UART8
#if defined(N32_UART8_SUPPRESS_ISR)
#error "N32_UART8_SUPPRESS_ISR is not supported by the serial driver"
#endif
/**
 * @brief   UART8 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_UART8_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  sd_lld_serve_interrupt(&SD8);
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Instance setup shared by all enabled USART/UART units.
 */
#define N32_SERIAL_INIT_SD(sd, dev, clk, notify, enreg, enbit,              \
                           rstreg, rstbit, irq, prio)                       \
  do {                                                                      \
    sdObjectInit(&(sd), NULL, notify);                                      \
    (sd).usart   = (dev);                                                   \
    (sd).clock   = (clk);                                                   \
    (sd).clk_reg = &(enreg);                                                \
    (sd).clk_bit = (enbit);                                                 \
    (sd).rst_reg = &(rstreg);                                               \
    (sd).rst_bit = (rstbit);                                                \
    nvicEnableVector(irq, prio);                                            \
  } while (false)

/**
 * @brief   Low level serial driver initialization.
 *
 * @notapi
 */
void sd_lld_init(void) {

#if N32_SERIAL_USE_USART1
  N32_SERIAL_INIT_SD(SD1, USART1, N32_PCLK2, notify1,
                     RCC->APB2PCLKEN, RCC_APB2PCLKEN_USART1EN,
                     RCC->APB2PRST,   RCC_APB2PRST_USART1RST,
                     USART1_IRQn, N32_SERIAL_USART1_IRQ_PRIORITY);
#endif
#if N32_SERIAL_USE_USART2
  N32_SERIAL_INIT_SD(SD2, USART2, N32_PCLK1, notify2,
                     RCC->APB1PCLKEN, RCC_APB1PCLKEN_USART2EN,
                     RCC->APB1PRST,   RCC_APB1PRST_USART2RST,
                     USART2_IRQn, N32_SERIAL_USART2_IRQ_PRIORITY);
#endif
#if N32_SERIAL_USE_USART3
  N32_SERIAL_INIT_SD(SD3, USART3, N32_PCLK1, notify3,
                     RCC->APB1PCLKEN, RCC_APB1PCLKEN_USART3EN,
                     RCC->APB1PRST,   RCC_APB1PRST_USART3RST,
                     USART3_IRQn, N32_SERIAL_USART3_IRQ_PRIORITY);
#endif
#if N32_SERIAL_USE_USART4
  N32_SERIAL_INIT_SD(SD4, USART4, N32_PCLK2, notify4,
                     RCC->APB2PCLKEN, RCC_APB2PCLKEN_USART4EN,
                     RCC->APB2PRST,   RCC_APB2PRST_USART4RST,
                     USART4_IRQn, N32_SERIAL_USART4_IRQ_PRIORITY);
#endif
#if N32_SERIAL_USE_UART5
  N32_SERIAL_INIT_SD(SD5, UART5, N32_PCLK1, notify5,
                     RCC->APB1PCLKEN, RCC_APB1PCLKEN_UART5EN,
                     RCC->APB1PRST,   RCC_APB1PRST_UART5RST,
                     UART5_IRQn, N32_SERIAL_UART5_IRQ_PRIORITY);
#endif
#if N32_SERIAL_USE_UART6
  N32_SERIAL_INIT_SD(SD6, UART6, N32_PCLK2, notify6,
                     RCC->APB2PCLKEN, RCC_APB2PCLKEN_UART6EN,
                     RCC->APB2PRST,   RCC_APB2PRST_UART6RST,
                     UART6_IRQn, N32_SERIAL_UART6_IRQ_PRIORITY);
#endif
#if N32_SERIAL_USE_UART7
  N32_SERIAL_INIT_SD(SD7, UART7, N32_PCLK2, notify7,
                     RCC->APB2PCLKEN, RCC_APB2PCLKEN_UART7EN,
                     RCC->APB2PRST,   RCC_APB2PRST_UART7RST,
                     UART7_IRQn, N32_SERIAL_UART7_IRQ_PRIORITY);
#endif
#if N32_SERIAL_USE_UART8
  N32_SERIAL_INIT_SD(SD8, UART8, N32_PCLK1, notify8,
                     RCC->APB1PCLKEN, RCC_APB1PCLKEN_UART8EN,
                     RCC->APB1PRST,   RCC_APB1PRST_UART8RST,
                     UART8_IRQn, N32_SERIAL_UART8_IRQ_PRIORITY);
#endif
}

/**
 * @brief   Low level serial driver configuration and (re)start.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration.
 *                      If this parameter is set to @p NULL then a default
 *                      configuration is used.
 *
 * @notapi
 */
void sd_lld_start(SerialDriver *sdp, const SerialConfig *config) {

  if (config == NULL) {
    config = &default_config;
  }

  if (sdp->state == SD_STOP) {
    usart_clock_enable(sdp);
  }
  usart_init(sdp, config);
}

/**
 * @brief   Low level serial driver stop.
 * @details De-initializes the USART and gates its peripheral clock.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 *
 * @notapi
 */
void sd_lld_stop(SerialDriver *sdp) {

  if (sdp->state == SD_READY) {
    usart_deinit(sdp->usart);
    usart_clock_disable(sdp);
  }
}

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the USART
 */
void sd_lld_serve_interrupt(SerialDriver *sdp) {
  USART_Module *u = sdp->usart;
  uint32_t ctrl1;
  uint32_t sts = u->STS;
  uint32_t guard = N32_SERIAL_RX_GUARD;

  /* Data available and/or RX errors.  Reading STS followed by DAT clears the
     error flags on this USART (same sequence as the STM32 USARTv1 driver). */
  osalSysLockFromISR();
  while (((sts & (USART_STS_RXDNE | USART_STS_OREF | USART_STS_NEF |
                  USART_STS_FEF | USART_STS_PEF)) != 0U) &&
         (guard-- != 0U)) {
    uint8_t b;

    if ((sts & (USART_STS_OREF | USART_STS_NEF |
                USART_STS_FEF | USART_STS_PEF)) != 0U) {
      set_error(sdp, sts);
    }
    b = (uint8_t)u->DAT & sdp->rxmask;
    if ((sts & USART_STS_RXDNE) != 0U) {
      sdIncomingDataI(sdp, b);
    }
    sts = u->STS;
  }
  osalSysUnlockFromISR();

  /* Caching CTRL1.*/
  ctrl1 = u->CTRL1;

  /* Transmission data register empty.*/
  if (((ctrl1 & USART_CTRL1_TXDEIEN) != 0U) &&
      ((sts & USART_STS_TXDE) != 0U)) {
    msg_t b;

    osalSysLockFromISR();
    b = oqGetI(&sdp->oqueue);
    if (b < MSG_OK) {
      chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
      ctrl1 &= ~USART_CTRL1_TXDEIEN;
    }
    else {
      u->DAT = (uint32_t)b;
    }
    osalSysUnlockFromISR();
  }

  /* Physical transmission end.*/
  if (((ctrl1 & USART_CTRL1_TXCIEN) != 0U) &&
      ((sts & USART_STS_TXC) != 0U)) {
    osalSysLockFromISR();
    if (oqIsEmptyI(&sdp->oqueue)) {
      chnAddFlagsI(sdp, CHN_TRANSMISSION_END);
      ctrl1 &= ~USART_CTRL1_TXCIEN;
    }
    osalSysUnlockFromISR();
  }

  /* Writing CTRL1 once.*/
  u->CTRL1 = ctrl1;
}

#endif /* HAL_USE_SERIAL */

/** @} */
