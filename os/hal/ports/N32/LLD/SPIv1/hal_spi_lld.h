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
 * @file    N32/LLD/SPIv1/hal_spi_lld.h
 * @brief   N32H47x/48x low level SPI driver header.
 *
 * @addtogroup SPI
 * @{
 */

#ifndef HAL_SPI_LLD_H
#define HAL_SPI_LLD_H

#if (HAL_USE_SPI == TRUE) || defined(__DOXYGEN__)

/* The official Nations CMSIS header defines SPI_Module, SPI1..SPI6, the RCC
   clock/reset bits and the IRQ numbers used by this driver. */
#include "n32h47x_48x.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Circular mode support flag.
 */
#define SPI_SUPPORTS_CIRCULAR               FALSE

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   SPI1 driver enable switch (default @p FALSE).
 */
#if !defined(N32_SPI_USE_SPI1) || defined(__DOXYGEN__)
#define N32_SPI_USE_SPI1                    FALSE
#endif

/** @brief SPI2 driver enable switch (default @p FALSE). */
#if !defined(N32_SPI_USE_SPI2) || defined(__DOXYGEN__)
#define N32_SPI_USE_SPI2                    FALSE
#endif

/** @brief SPI3 driver enable switch (default @p FALSE). */
#if !defined(N32_SPI_USE_SPI3) || defined(__DOXYGEN__)
#define N32_SPI_USE_SPI3                    FALSE
#endif

/** @brief SPI4 driver enable switch (default @p FALSE). */
#if !defined(N32_SPI_USE_SPI4) || defined(__DOXYGEN__)
#define N32_SPI_USE_SPI4                    FALSE
#endif

/** @brief SPI5 driver enable switch (default @p FALSE). */
#if !defined(N32_SPI_USE_SPI5) || defined(__DOXYGEN__)
#define N32_SPI_USE_SPI5                    FALSE
#endif

/** @brief SPI6 driver enable switch (default @p FALSE). */
#if !defined(N32_SPI_USE_SPI6) || defined(__DOXYGEN__)
#define N32_SPI_USE_SPI6                    FALSE
#endif

/**
 * @name    IRQ priority defaults
 * @{
 */
#if !defined(N32_SPI_SPI1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SPI_SPI1_IRQ_PRIORITY           12
#endif
#if !defined(N32_SPI_SPI2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SPI_SPI2_IRQ_PRIORITY           12
#endif
#if !defined(N32_SPI_SPI3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SPI_SPI3_IRQ_PRIORITY           12
#endif
#if !defined(N32_SPI_SPI4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SPI_SPI4_IRQ_PRIORITY           12
#endif
#if !defined(N32_SPI_SPI5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SPI_SPI5_IRQ_PRIORITY           12
#endif
#if !defined(N32_SPI_SPI6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SPI_SPI6_IRQ_PRIORITY           12
#endif
/** @} */
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (N32_SPI_USE_SPI1 == FALSE) && (N32_SPI_USE_SPI2 == FALSE) &&            \
    (N32_SPI_USE_SPI3 == FALSE) && (N32_SPI_USE_SPI4 == FALSE) &&            \
    (N32_SPI_USE_SPI5 == FALSE) && (N32_SPI_USE_SPI6 == FALSE)
#error "SPI driver activated but no SPI peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Low level fields of the SPI driver structure.
 */
#define spi_lld_driver_fields                                                  \
  SPI_Module               *spi;                                               \
  volatile uint32_t        *clk_reg;                                           \
  uint32_t                  clk_bit;                                           \
  volatile uint32_t        *rst_reg;                                           \
  uint32_t                  rst_bit;                                           \
  IRQn_Type                 irq;                                               \
  uint8_t                   irq_prio;                                          \
  const uint8_t            *txptr;                                             \
  uint8_t                  *rxptr;                                             \
  size_t                    txcnt;                                              \
  size_t                    rxcnt

/**
 * @brief   Low level fields of the SPI configuration structure.
 * @note    @p cr1 carries the STM32-style bits written by QMK's generic
 *          spi_master.c (BR/CPOL/CPHA/LSBFIRST).  They are mapped onto the
 *          N32 CTRL1 bit positions in platforms/chibios/chibios_config.h.
 *          @p cr2 is reserved for future CTRL2 options and must be zero.
 */
#define spi_lld_config_fields                                                  \
  uint16_t                  cr1;                                               \
  uint16_t                  cr2

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (N32_SPI_USE_SPI1 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID1;
#endif
#if (N32_SPI_USE_SPI2 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID2;
#endif
#if (N32_SPI_USE_SPI3 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID3;
#endif
#if (N32_SPI_USE_SPI4 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID4;
#endif
#if (N32_SPI_USE_SPI5 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID5;
#endif
#if (N32_SPI_USE_SPI6 == TRUE) && !defined(__DOXYGEN__)
extern SPIDriver SPID6;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void spi_lld_init(void);
  void spi_lld_start(SPIDriver *spip);
  void spi_lld_stop(SPIDriver *spip);
  void spi_lld_select(SPIDriver *spip);
  void spi_lld_unselect(SPIDriver *spip);
  void spi_lld_ignore(SPIDriver *spip, size_t n);
  void spi_lld_exchange(SPIDriver *spip, size_t n,
                        const void *txbuf, void *rxbuf);
  void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf);
  void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf);
  uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SPI == TRUE */

#endif /* HAL_SPI_LLD_H */

/** @} */
