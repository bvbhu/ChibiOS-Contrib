/*
    ChibiOS - Copyright (C) 2026 bvbhu (N32H48x port for vial-qmk-stg-n32h481)

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
 * @file    N32/LLD/SPIv1/hal_spi_lld.c
 * @brief   N32H47x/48x low level SPI driver source.
 *
 * @details Interrupt driven, 8-bit frames, master mode, software NSS.
 *          No DMA is required: the TXE/RXNE interrupts move one frame at a
 *          time, which is a good fit for the short transfers QMK issues.
 *
 * @addtogroup SPI
 * @{
 */

#include "hal.h"

#if (HAL_USE_SPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* N32 keeps the SPI enable bit in CTRL2 (not CTRL1 as on STM32) and uses a
   separate SPI_I2S_CFGR register to select between SPI and I2S mode. */
#define N32_SPI_I2S_MODE_BIT                ((uint16_t)0x0002U)

#define N32_SPI_CTRL1_CONFIG_MASK          (SPI_CTRL1_BR | SPI_CLKPOL |       \
                                           SPI_CLKPHA | SPI_CTRL1_LSBFF)

#define N32_SPI_CTRL1_MASTER_SOFT_NSS      (SPI_CTRL1_MSEL |                 \
                                           SPI_CTRL1_SSMEN |                 \
                                           SPI_CTRL1_SSEL)

#define N32_SPI_XFER_IRQ_BITS              (SPI_CTRL2_TEINTEN |             \
                                           SPI_CTRL2_RNEINTEN)

#define N32_SPI_ERROR_FLAGS                (SPI_STS_CRCERR | SPI_STS_MODERR |\
                                           SPI_STS_OVER | SPI_STS_UNDER)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if (N32_SPI_USE_SPI1 == TRUE) || defined(__DOXYGEN__)
/** @brief SPI1 driver identifier. */
SPIDriver SPID1;
#endif
#if (N32_SPI_USE_SPI2 == TRUE) || defined(__DOXYGEN__)
/** @brief SPI2 driver identifier. */
SPIDriver SPID2;
#endif
#if (N32_SPI_USE_SPI3 == TRUE) || defined(__DOXYGEN__)
/** @brief SPI3 driver identifier. */
SPIDriver SPID3;
#endif
#if (N32_SPI_USE_SPI4 == TRUE) || defined(__DOXYGEN__)
/** @brief SPI4 driver identifier. */
SPIDriver SPID4;
#endif
#if (N32_SPI_USE_SPI5 == TRUE) || defined(__DOXYGEN__)
/** @brief SPI5 driver identifier. */
SPIDriver SPID5;
#endif
#if (N32_SPI_USE_SPI6 == TRUE) || defined(__DOXYGEN__)
/** @brief SPI6 driver identifier. */
SPIDriver SPID6;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common interrupt service routine for all SPI instances.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
static void spi_lld_serve_interrupt(SPIDriver *spip) {
  SPI_Module *spi = spip->spi;
  uint16_t    sts = spi->STS;

  /* Error flags are cleared by the STS read followed by a DAT read.  They
     are not fatal for a keyboard: drop the frame and keep the transfer
     going instead of leaving the bus stuck with interrupts enabled. */
  if ((sts & N32_SPI_ERROR_FLAGS) != 0U) {
    (void)spi->DAT;
  }

  /* Receive first: reading DAT clears RNE before the next frame is queued. */
  if ((sts & SPI_STS_RNE) != 0U) {
    uint16_t data = spi->DAT;
    if (spip->rxcnt > 0U) {
      if (spip->rxptr != NULL) {
        *spip->rxptr++ = (uint8_t)data;
      }
      spip->rxcnt--;
    }
  }

  if (((sts & SPI_STS_TE) != 0U) && (spip->txcnt > 0U)) {
    uint16_t data = 0xFFU;
    if (spip->txptr != NULL) {
      data = *spip->txptr++;
    }
    spi->DAT = data;
    spip->txcnt--;

    /* Once the last frame has been queued only RXNE is needed to finish. */
    if (spip->txcnt == 0U) {
      spi->CTRL2 &= (uint16_t)~SPI_CTRL2_TEINTEN;
    }
  }

  if ((spip->state == SPI_ACTIVE) && (spip->txcnt == 0U) &&
      (spip->rxcnt == 0U)) {
    spi->CTRL2 &= (uint16_t)~N32_SPI_XFER_IRQ_BITS;
    _spi_isr_code(spip);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if (N32_SPI_USE_SPI1 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   SPI1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_SPI1_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  spi_lld_serve_interrupt(&SPID1);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if (N32_SPI_USE_SPI2 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   SPI2/I2S2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_SPI2_I2S2_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  spi_lld_serve_interrupt(&SPID2);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if (N32_SPI_USE_SPI3 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   SPI3/I2S3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_SPI3_I2S3_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  spi_lld_serve_interrupt(&SPID3);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if (N32_SPI_USE_SPI4 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   SPI4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_SPI4_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  spi_lld_serve_interrupt(&SPID4);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if (N32_SPI_USE_SPI5 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   SPI5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_SPI5_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  spi_lld_serve_interrupt(&SPID5);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if (N32_SPI_USE_SPI6 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   SPI6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(N32_SPI6_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  spi_lld_serve_interrupt(&SPID6);
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Instance setup shared by all enabled SPI units.
 */
#define N32_SPI_INIT_SD(sd, dev, clkr, clkbit, rstr, rstbit, irqn, prio)      \
  do {                                                                       \
    spiObjectInit(&(sd));                                                     \
    (sd).spi      = (dev);                                                    \
    (sd).clk_reg  = &(clkr);                                                  \
    (sd).clk_bit  = (clkbit);                                                 \
    (sd).rst_reg  = &(rstr);                                                  \
    (sd).rst_bit  = (rstbit);                                                 \
    (sd).irq      = (irqn);                                                   \
    (sd).irq_prio = (uint8_t)(prio);                                          \
    (sd).txptr    = NULL;                                                      \
    (sd).rxptr    = NULL;                                                      \
    (sd).txcnt    = 0U;                                                       \
    (sd).rxcnt    = 0U;                                                       \
  } while (false)

/**
 * @brief   Low level SPI driver initialization.
 *
 * @notapi
 */
void spi_lld_init(void) {

#if N32_SPI_USE_SPI1
  N32_SPI_INIT_SD(SPID1, SPI1, RCC->APB2PCLKEN, RCC_APB2PCLKEN_SPI1EN,
                  RCC->APB2PRST, RCC_APB2PRST_SPI1RST,
                  SPI1_IRQn, N32_SPI_SPI1_IRQ_PRIORITY);
#endif
#if N32_SPI_USE_SPI2
  N32_SPI_INIT_SD(SPID2, SPI2, RCC->APB1PCLKEN, RCC_APB1PCLKEN_SPI2EN,
                  RCC->APB1PRST, RCC_APB1PRST_SPI2RST,
                  SPI2_I2S2_IRQn, N32_SPI_SPI2_IRQ_PRIORITY);
#endif
#if N32_SPI_USE_SPI3
  N32_SPI_INIT_SD(SPID3, SPI3, RCC->APB1PCLKEN, RCC_APB1PCLKEN_SPI3EN,
                  RCC->APB1PRST, RCC_APB1PRST_SPI3RST,
                  SPI3_I2S3_IRQn, N32_SPI_SPI3_IRQ_PRIORITY);
#endif
#if N32_SPI_USE_SPI4
  N32_SPI_INIT_SD(SPID4, SPI4, RCC->APB2PCLKEN, RCC_APB2PCLKEN_SPI4EN,
                  RCC->APB2PRST, RCC_APB2PRST_SPI4RST,
                  SPI4_IRQn, N32_SPI_SPI4_IRQ_PRIORITY);
#endif
#if N32_SPI_USE_SPI5
  N32_SPI_INIT_SD(SPID5, SPI5, RCC->APB2PCLKEN, RCC_APB2PCLKEN_SPI5EN,
                  RCC->APB2PRST, RCC_APB2PRST_SPI5RST,
                  SPI5_IRQn, N32_SPI_SPI5_IRQ_PRIORITY);
#endif
#if N32_SPI_USE_SPI6
  N32_SPI_INIT_SD(SPID6, SPI6, RCC->APB2PCLKEN, RCC_APB2PCLKEN_SPI6EN,
                  RCC->APB2PRST, RCC_APB2PRST_SPI6RST,
                  SPI6_IRQn, N32_SPI_SPI6_IRQ_PRIORITY);
#endif
}

/**
 * @brief   Configures and activates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_start(SPIDriver *spip) {

  if (spip->state == SPI_STOP) {
    /* Reset then release the unit and enable its bus clock. */
    *spip->rst_reg |= spip->rst_bit;
    *spip->rst_reg &= ~spip->rst_bit;
    *spip->clk_reg |= spip->clk_bit;
    nvicEnableVector(spip->irq, spip->irq_prio);
  }

  /* CTRL2.SPIEN must be clear while CTRL1 is programmed.  FIFO mode stays
     off so TE/RNE describe a single frame, which is what the ISR expects. */
  spip->spi->CTRL2 = 0U;
  spip->spi->CTRL1 = (uint16_t)((spip->config->cr1 &
                                 N32_SPI_CTRL1_CONFIG_MASK) |
                                N32_SPI_CTRL1_MASTER_SOFT_NSS);
  spip->spi->SPI_I2S_CFGR &= (uint16_t)~N32_SPI_I2S_MODE_BIT;
  spip->spi->CRCPOLY = 7U;

  /* A stale RNE from a previous transfer would fire immediately. */
  (void)spip->spi->STS;
  (void)spip->spi->DAT;

  (void)spip->config->cr2;
  spip->spi->CTRL2 = SPI_CTRL2_SPIEN;
}

/**
 * @brief   Deactivates the SPI peripheral.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_stop(SPIDriver *spip) {

  if (spip->state == SPI_READY) {
    spip->spi->CTRL2 = 0U;
    nvicDisableVector(spip->irq);
    *spip->clk_reg &= ~spip->clk_bit;
  }
}

/**
 * @brief   Asserts the slave select signal.
 * @details Nothing to do here: the portable layer drives CS itself through
 *          the spiSelectI() macro, which is generated from SPI_SELECT_MODE
 *          (PAD by default, LINE/PORT/NONE also supported).  Only
 *          SPI_SELECT_MODE_LLD would require an implementation below.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_select(SPIDriver *spip) {
  (void)spip;
}

/**
 * @brief   Deasserts the slave select signal.
 * @details See @p spi_lld_select().
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
void spi_lld_unselect(SPIDriver *spip) {
  (void)spip;
}

/**
 * @brief   Ignores data on the SPI bus.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be ignored
 *
 * @notapi
 */
void spi_lld_ignore(SPIDriver *spip, size_t n) {

  spi_lld_exchange(spip, n, NULL, NULL);
}

/**
 * @brief   Exchanges data on the SPI bus.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be exchanged
 * @param[in] txbuf     the pointer to the transmit buffer
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void spi_lld_exchange(SPIDriver *spip, size_t n,
                      const void *txbuf, void *rxbuf) {

  spip->txptr = (const uint8_t *)txbuf;
  spip->rxptr = (uint8_t *)rxbuf;
  spip->txcnt = n;
  spip->rxcnt = n;

  /* Drop anything left over from the previous transfer. */
  (void)spip->spi->STS;
  (void)spip->spi->DAT;

  /* TXE is already set, so the ISR queues the very first frame. */
  spip->spi->CTRL2 |= N32_SPI_XFER_IRQ_BITS;
}

/**
 * @brief   Sends data over the SPI bus.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf) {

  spi_lld_exchange(spip, n, txbuf, NULL);
}

/**
 * @brief   Receives data from the SPI bus.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to receive
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf) {

  spi_lld_exchange(spip, n, NULL, rxbuf);
}

/**
 * @brief   Exchanges one frame using a polled wait.
 * @note    Only valid while no interrupt driven transfer is in progress.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] frame     the data frame to send over the SPI bus
 * @return              The received data frame from the SPI bus.
 */
uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame) {

  while ((spip->spi->STS & SPI_STS_TE) == 0U) {
    ;
  }
  spip->spi->DAT = frame;
  while ((spip->spi->STS & SPI_STS_RNE) == 0U) {
    ;
  }
  return (uint16_t)spip->spi->DAT;
}

#endif /* HAL_USE_SPI == TRUE */

/** @} */
