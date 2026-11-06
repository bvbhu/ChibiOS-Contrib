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
 * @file    N32/LLD/USARTv1/hal_serial_lld.h
 * @brief   N32H48x low level serial driver header.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef HAL_SERIAL_LLD_H
#define HAL_SERIAL_LLD_H

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

/* The official Nations CMSIS header defines USART_Module, USART1..UART8,
   the RCC clock/reset bits and the IRQ numbers used by this driver. */
#include "n32h47x_48x.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   USART1 driver enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(N32_SERIAL_USE_USART1) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_USART1               FALSE
#endif

/** @brief USART2 driver enable switch (default @p FALSE). */
#if !defined(N32_SERIAL_USE_USART2) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_USART2               FALSE
#endif

/** @brief USART3 driver enable switch (default @p FALSE). */
#if !defined(N32_SERIAL_USE_USART3) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_USART3               FALSE
#endif

/** @brief USART4 driver enable switch (default @p FALSE). */
#if !defined(N32_SERIAL_USE_USART4) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_USART4               FALSE
#endif

/** @brief UART5 driver enable switch (default @p FALSE). */
#if !defined(N32_SERIAL_USE_UART5) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_UART5                FALSE
#endif

/** @brief UART6 driver enable switch (default @p FALSE). */
#if !defined(N32_SERIAL_USE_UART6) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_UART6                FALSE
#endif

/** @brief UART7 driver enable switch (default @p FALSE). */
#if !defined(N32_SERIAL_USE_UART7) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_UART7                FALSE
#endif

/** @brief UART8 driver enable switch (default @p FALSE). */
#if !defined(N32_SERIAL_USE_UART8) || defined(__DOXYGEN__)
#define N32_SERIAL_USE_UART8                FALSE
#endif

/**
 * @name    IRQ priority settings
 * @note    The default priority for all instances is 12.
 * @{
 */
#if !defined(N32_SERIAL_USART1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_USART1_IRQ_PRIORITY       12
#endif
#if !defined(N32_SERIAL_USART2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_USART2_IRQ_PRIORITY       12
#endif
#if !defined(N32_SERIAL_USART3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_USART3_IRQ_PRIORITY       12
#endif
#if !defined(N32_SERIAL_USART4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_USART4_IRQ_PRIORITY       12
#endif
#if !defined(N32_SERIAL_UART5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_UART5_IRQ_PRIORITY        12
#endif
#if !defined(N32_SERIAL_UART6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_UART6_IRQ_PRIORITY        12
#endif
#if !defined(N32_SERIAL_UART7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_UART7_IRQ_PRIORITY        12
#endif
#if !defined(N32_SERIAL_UART8_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_SERIAL_UART8_IRQ_PRIORITY        12
#endif
/** @} */
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if N32_SERIAL_USE_USART1 && !N32_HAS_USART1
#error "USART1 not present in the selected device"
#endif
#if N32_SERIAL_USE_USART2 && !N32_HAS_USART2
#error "USART2 not present in the selected device"
#endif
#if N32_SERIAL_USE_USART3 && !N32_HAS_USART3
#error "USART3 not present in the selected device"
#endif
#if N32_SERIAL_USE_USART4 && !N32_HAS_USART4
#error "USART4 not present in the selected device"
#endif
#if N32_SERIAL_USE_UART5 && !N32_HAS_UART5
#error "UART5 not present in the selected device"
#endif
#if N32_SERIAL_USE_UART6 && !N32_HAS_UART6
#error "UART6 not present in the selected device"
#endif
#if N32_SERIAL_USE_UART7 && !N32_HAS_UART7
#error "UART7 not present in the selected device"
#endif
#if N32_SERIAL_USE_UART8 && !N32_HAS_UART8
#error "UART8 not present in the selected device"
#endif

#if !N32_SERIAL_USE_USART1 && !N32_SERIAL_USE_USART2 &&                    \
    !N32_SERIAL_USE_USART3 && !N32_SERIAL_USE_USART4 &&                    \
    !N32_SERIAL_USE_UART5  && !N32_SERIAL_USE_UART6  &&                    \
    !N32_SERIAL_USE_UART7  && !N32_SERIAL_USE_UART8
#error "SERIAL driver activated but no USART/UART peripheral assigned"
#endif

#if N32_SERIAL_USE_USART1 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_USART1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to USART1"
#endif
#if N32_SERIAL_USE_USART2 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_USART2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to USART2"
#endif
#if N32_SERIAL_USE_USART3 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_USART3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to USART3"
#endif
#if N32_SERIAL_USE_USART4 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_USART4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to USART4"
#endif
#if N32_SERIAL_USE_UART5 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_UART5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART5"
#endif
#if N32_SERIAL_USE_UART6 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_UART6_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART6"
#endif
#if N32_SERIAL_USE_UART7 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_UART7_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART7"
#endif
#if N32_SERIAL_USE_UART8 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(N32_SERIAL_UART8_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART8"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   N32H48x Serial Driver configuration structure.
 * @details An instance of this structure must be passed to @p sdStart()
 *          in order to configure and start a serial driver operation.
 * @note    The three register values are written (OR-ed with the bits the
 *          driver enforces) into CTRL1, CTRL2 and CTRL3.  The default
 *          configuration is 38400 8N1 with one stop bit.
 */
typedef struct hal_serial_config {
  /**
   * @brief Bit rate.
   */
  uint32_t                  speed;
  /* End of the mandatory fields.*/
  /**
   * @brief Initialization value for the CTRL1 register.
   */
  uint32_t                  ctrl1;
  /**
   * @brief Initialization value for the CTRL2 register.
   */
  uint32_t                  ctrl2;
  /**
   * @brief Initialization value for the CTRL3 register.
   */
  uint32_t                  ctrl3;
} SerialConfig;

/**
 * @brief   @p SerialDriver specific data.
 */
#define _serial_driver_data                                                 \
  _base_asynchronous_channel_data                                           \
  /* Driver state.*/                                                        \
  sdstate_t                 state;                                          \
  /* Input queue.*/                                                         \
  input_queue_t             iqueue;                                         \
  /* Output queue.*/                                                        \
  output_queue_t            oqueue;                                         \
  /* Input circular buffer.*/                                               \
  uint8_t                   ib[SERIAL_BUFFERS_SIZE];                        \
  /* Output circular buffer.*/                                              \
  uint8_t                   ob[SERIAL_BUFFERS_SIZE];                        \
  /* End of the mandatory fields.*/                                         \
  /* Pointer to the USART registers block.*/                                \
  USART_Module              *usart;                                         \
  /* Clock frequency of the APB bus feeding the USART/UART.*/               \
  uint32_t                  clock;                                          \
  /* RCC clock-enable / reset register and bit for this instance.*/         \
  volatile uint32_t         *clk_reg;                                       \
  uint32_t                  clk_bit;                                        \
  volatile uint32_t         *rst_reg;                                       \
  uint32_t                  rst_bit;                                        \
  /* Mask to be applied on received frames.*/                               \
  uint8_t                   rxmask;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    CTRL2 stop-bit values
 * @note    N32H48x keeps STOP[1:0] in bits 6:5 (not bits 13:12 as on AT32).
 * @{
 */
#define N32_USART_CTRL2_STOP1_BITS          (0U << 5)
#define N32_USART_CTRL2_STOP0P5_BITS        (1U << 5)
#define N32_USART_CTRL2_STOP2_BITS          (2U << 5)
#define N32_USART_CTRL2_STOP1P5_BITS        (3U << 5)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if N32_SERIAL_USE_USART1 && !defined(__DOXYGEN__)
extern SerialDriver SD1;
#endif
#if N32_SERIAL_USE_USART2 && !defined(__DOXYGEN__)
extern SerialDriver SD2;
#endif
#if N32_SERIAL_USE_USART3 && !defined(__DOXYGEN__)
extern SerialDriver SD3;
#endif
#if N32_SERIAL_USE_USART4 && !defined(__DOXYGEN__)
extern SerialDriver SD4;
#endif
#if N32_SERIAL_USE_UART5 && !defined(__DOXYGEN__)
extern SerialDriver SD5;
#endif
#if N32_SERIAL_USE_UART6 && !defined(__DOXYGEN__)
extern SerialDriver SD6;
#endif
#if N32_SERIAL_USE_UART7 && !defined(__DOXYGEN__)
extern SerialDriver SD7;
#endif
#if N32_SERIAL_USE_UART8 && !defined(__DOXYGEN__)
extern SerialDriver SD8;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sd_lld_init(void);
  void sd_lld_start(SerialDriver *sdp, const SerialConfig *config);
  void sd_lld_stop(SerialDriver *sdp);
  void sd_lld_serve_interrupt(SerialDriver *sdp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SERIAL */

#endif /* HAL_SERIAL_LLD_H */

/** @} */
