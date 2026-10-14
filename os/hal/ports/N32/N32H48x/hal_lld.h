/*
    ChibiOS - Copyright (C) 2026 bvbhu (vial-qmk-stg-n32h481 fork)

    N32H48x HAL low level driver header (NSING N32H481REL7K).

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

#ifndef _HAL_LLD_H_
#define _HAL_LLD_H_

#include "n32_registry.h"

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
#endif

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @brief   Platform name.
 */
#define PLATFORM_NAME                  "N32H48x"

/**
 * @brief   N32H48x device type.
 */
#if !defined(N32H481RE) || defined(__DOXYGEN__)
#define N32H481RE
#endif

/*===========================================================================*/
/* Platform clocks.                                                          */
/*===========================================================================*/

#define N32_SYSCLK_MAX                240000000
#define N32_HCLK_MAX                  240000000
#define N32_PCLK1_MAX                 120000000
#define N32_PCLK2_MAX                 120000000
#define N32_PLLIN_MIN                 1000000
#define N32_PLLIN_MAX                 24000000
#define N32_PLLOUT_MIN                48000000
#define N32_PLLOUT_MAX                240000000
#define N32_HICKCLK                   8000000
#define N32_HICKCLK_MIN               5000000
#define N32_HICKCLK_MAX               10000000
#define N32_LICKCLK                   32768
#define N32_HEXTCLK_MIN               4000000
#define N32_HEXTCLK_MAX               25000000
#define N32_LPFC_ENABLE               TRUE

/*===========================================================================*/
/* Clock and bus settings.                                                   */
/*===========================================================================*/

/**
 * @brief   System clock switch (SCLKSW field).
 */
#define N32_SCLKSW_HSI                (0 << 27)
#define N32_SCLKSW_HSE                (1 << 27)
#define N32_SCLKSW_PLL                (2 << 27)
#define N32_SCLKSW_USBHS240M          (5 << 27)
#define N32_SCLKSW_EXTPLL             (6 << 27)

/**
 * @brief   PLL clock source (PLLSRC field).
 */
#define N32_PLLSRC_HSI                (0 << 0)
#define N32_PLLSRC_HSE                (1 << 0)

/**
 * @brief   AHB prescaler (AHBPRES field).
 */
#define N32_AHBDIV_DIV1               (0 << 16)
#define N32_AHBDIV_DIV2               (8 << 16)
#define N32_AHBDIV_DIV4               (9 << 16)
#define N32_AHBDIV_DIV8               (10 << 16)
#define N32_AHBDIV_DIV16              (11 << 16)
#define N32_AHBDIV_DIV64              (12 << 16)
#define N32_AHBDIV_DIV128             (13 << 16)
#define N32_AHBDIV_DIV256             (14 << 16)
#define N32_AHBDIV_DIV512             (15 << 16)

/**
 * @brief   APB1 prescaler (APB1PRES field).
 */
#define N32_APB1DIV_DIV1              (0 << 11)
#define N32_APB1DIV_DIV2              (4 << 11)
#define N32_APB1DIV_DIV4              (5 << 11)
#define N32_APB1DIV_DIV8              (6 << 11)
#define N32_APB1DIV_DIV16             (7 << 11)

/**
 * @brief   APB2 prescaler (APB2PRES field).
 */
#define N32_APB2DIV_DIV1              (0 << 14)
#define N32_APB2DIV_DIV2              (4 << 14)
#define N32_APB2DIV_DIV4              (5 << 14)
#define N32_APB2DIV_DIV8              (6 << 14)
#define N32_APB2DIV_DIV16             (7 << 14)

/*===========================================================================*/
/* Clock and bus settings.                                                   */
/*===========================================================================*/

/**
 * @brief   N32 clock sources.
 */
#define N32_SCLKSEL_HSI               0
#define N32_SCLKSEL_HSE               1
#define N32_SCLKSEL_PLL               2
#define N32_SCLKSEL_USBHS240M         3
#define N32_SCLKSEL_EXTPLL            4

/*===========================================================================*/
/* Clock and bus settings.                                                   */
/*===========================================================================*/

#if !defined(N32_NO_INIT) || defined(__DOXYGEN__)
#define N32_NO_INIT                   FALSE
#endif

#if !defined(N32_HICK_ENABLED) || defined(__DOXYGEN__)
#define N32_HICK_ENABLED              TRUE
#endif

#if !defined(N32_LICK_ENABLED) || defined(__DOXYGEN__)
#define N32_LICK_ENABLED              FALSE
#endif

#if !defined(N32_HEXT_ENABLED) || defined(__DOXYGEN__)
#define N32_HEXT_ENABLED              FALSE
#endif

#if !defined(N32_SCLKSEL) || defined(__DOXYGEN__)
#define N32_SCLKSEL                   N32_SCLKSEL_PLL
#endif

#if !defined(N32_PLLSRC) || defined(__DOXYGEN__)
#define N32_PLLSRC                    N32_PLLSRC_HSI
#endif

#if !defined(N32_PLLMULFCT_VALUE) || defined(__DOXYGEN__)
#define N32_PLLMULFCT_VALUE           30
#endif

#if !defined(N32_AHBDIV) || defined(__DOXYGEN__)
#define N32_AHBDIV                    N32_AHBDIV_DIV1
#endif

#if !defined(N32_APB1DIV) || defined(__DOXYGEN__)
#define N32_APB1DIV                   N32_APB1DIV_DIV2
#endif

#if !defined(N32_APB2DIV) || defined(__DOXYGEN__)
#define N32_APB2DIV                   N32_APB2DIV_DIV2
#endif

#if !defined(N32_PLLINPRES_VALUE) || defined(__DOXYGEN__)
#define N32_PLLINPRES_VALUE           1
#endif

#if !defined(N32_PLLOUTPRES_VALUE) || defined(__DOXYGEN__)
#define N32_PLLOUTPRES_VALUE          1
#endif

/*===========================================================================*/
/* Clock and bus settings.                                                   */
/*===========================================================================*/

/**
 * @brief   PLL multiplication field value.
 */
#if ((N32_PLLMULFCT_VALUE >= 4) && (N32_PLLMULFCT_VALUE <= 76)) ||            \
    defined(__DOXYGEN__)
#define N32_PLLMULFCT                 (((N32_PLLMULFCT_VALUE - 4) / 2) << 3)
#else
#error "invalid N32_PLLMULFCT_VALUE value specified"
#endif

/**
 * @brief   PLL input clock frequency.
 */
#if (N32_PLLSRC == N32_PLLSRC_HSE) || defined(__DOXYGEN__)
#define N32_PLLCLKIN                  N32_HEXTCLK
#elif N32_PLLSRC == N32_PLLSRC_HSI
#define N32_PLLCLKIN                  N32_HICKCLK
#else
#error "invalid N32_PLLSRC value specified"
#endif

/* PLL input frequency range check.*/
#if (N32_PLLCLKIN < N32_PLLIN_MIN) || (N32_PLLCLKIN > N32_PLLIN_MAX)
#error "N32_PLLCLKIN outside acceptable range (N32_PLLIN_MIN...N32_PLLIN_MAX)"
#endif

/**
 * @brief   PLL output clock frequency.
 */
#define N32_PLLCLKOUT                 (N32_PLLCLKIN * N32_PLLMULFCT_VALUE)

/* PLL output frequency range check.*/
#if (N32_PLLCLKOUT < N32_PLLOUT_MIN) || (N32_PLLCLKOUT > N32_PLLOUT_MAX)
#error "N32_PLLCLKOUT outside acceptable range (N32_PLLOUT_MIN...N32_PLLOUT_MAX)"
#endif

/**
 * @brief   USBHS clock frequency (USBHSCLK).
 * @note    The USBHS clock is derived from the main PLL via the USBPLL
 *          prescaler (RCC_CFG3_USBPLLPRES). The default 240MHz PLL with
 *          the DIV5 prescaler yields 48MHz.
 */
#if (N32_PLLCLKOUT == 240000000) || defined(__DOXYGEN__)
#define N32_USBCLK                    (N32_PLLCLKOUT / 5)
#elif (N32_PLLCLKOUT == 192000000)
#define N32_USBCLK                    (N32_PLLCLKOUT / 4)
#elif (N32_PLLCLKOUT == 144000000)
#define N32_USBCLK                    (N32_PLLCLKOUT / 3)
#elif (N32_PLLCLKOUT == 96000000)
#define N32_USBCLK                    (N32_PLLCLKOUT / 2)
#elif (N32_PLLCLKOUT == 48000000)
#define N32_USBCLK                    (N32_PLLCLKOUT / 1)
#else
#error "N32_USBCLK: unsupported N32_PLLCLKOUT value, cannot derive USB 48MHz clock"
#endif

/**
 * @brief   System clock source.
 */
#if (N32_SCLKSEL == N32_SCLKSEL_PLL) || defined(__DOXYGEN__)
#define N32_SYSCLK                    N32_PLLCLKOUT
#elif N32_SCLKSEL == N32_SCLKSEL_HSI
#define N32_SYSCLK                    N32_HICKCLK
#elif N32_SCLKSEL == N32_SCLKSEL_HSE
#define N32_SYSCLK                    N32_HEXTCLK
#else
#error "invalid N32_SCLKSEL value specified"
#endif

/* Check on the system clock.*/
#if N32_SYSCLK > N32_SYSCLK_MAX
#error "N32_SYSCLK exceeding maximum frequency (N32_SYSCLK_MAX)"
#endif

/**
 * @brief   AHB frequency.
 */
#if (N32_AHBDIV == N32_AHBDIV_DIV1) || defined(__DOXYGEN__)
#define N32_HCLK                      (N32_SYSCLK / 1)
#elif N32_AHBDIV == N32_AHBDIV_DIV2
#define N32_HCLK                      (N32_SYSCLK / 2)
#elif N32_AHBDIV == N32_AHBDIV_DIV4
#define N32_HCLK                      (N32_SYSCLK / 4)
#elif N32_AHBDIV == N32_AHBDIV_DIV8
#define N32_HCLK                      (N32_SYSCLK / 8)
#elif N32_AHBDIV == N32_AHBDIV_DIV16
#define N32_HCLK                      (N32_SYSCLK / 16)
#elif N32_AHBDIV == N32_AHBDIV_DIV64
#define N32_HCLK                      (N32_SYSCLK / 64)
#elif N32_AHBDIV == N32_AHBDIV_DIV128
#define N32_HCLK                      (N32_SYSCLK / 128)
#elif N32_AHBDIV == N32_AHBDIV_DIV256
#define N32_HCLK                      (N32_SYSCLK / 256)
#elif N32_AHBDIV == N32_AHBDIV_DIV512
#define N32_HCLK                      (N32_SYSCLK / 512)
#else
#error "invalid N32_AHBDIV value specified"
#endif

/* AHB frequency check.*/
#if N32_HCLK > N32_HCLK_MAX
#error "N32_HCLK exceeding maximum frequency (N32_HCLK_MAX)"
#endif

/**
 * @brief   APB1 frequency.
 */
#if (N32_APB1DIV == N32_APB1DIV_DIV1) || defined(__DOXYGEN__)
#define N32_PCLK1                     (N32_HCLK / 1)
#elif N32_APB1DIV == N32_APB1DIV_DIV2
#define N32_PCLK1                     (N32_HCLK / 2)
#elif N32_APB1DIV == N32_APB1DIV_DIV4
#define N32_PCLK1                     (N32_HCLK / 4)
#elif N32_APB1DIV == N32_APB1DIV_DIV8
#define N32_PCLK1                     (N32_HCLK / 8)
#elif N32_APB1DIV == N32_APB1DIV_DIV16
#define N32_PCLK1                     (N32_HCLK / 16)
#else
#error "invalid N32_APB1DIV value specified"
#endif

/* APB1 frequency check.*/
#if N32_PCLK1 > N32_PCLK1_MAX
#error "N32_PCLK1 exceeding maximum frequency (N32_PCLK1_MAX)"
#endif

/**
 * @brief   APB2 frequency.
 */
#if (N32_APB2DIV == N32_APB2DIV_DIV1) || defined(__DOXYGEN__)
#define N32_PCLK2                     (N32_HCLK / 1)
#elif N32_APB2DIV == N32_APB2DIV_DIV2
#define N32_PCLK2                     (N32_HCLK / 2)
#elif N32_APB2DIV == N32_APB2DIV_DIV4
#define N32_PCLK2                     (N32_HCLK / 4)
#elif N32_APB2DIV == N32_APB2DIV_DIV8
#define N32_PCLK2                     (N32_HCLK / 8)
#elif N32_APB2DIV == N32_APB2DIV_DIV16
#define N32_PCLK2                     (N32_HCLK / 16)
#else
#error "invalid N32_APB2DIV value specified"
#endif

/* APB2 frequency check.*/
#if N32_PCLK2 > N32_PCLK2_MAX
#error "N32_PCLK2 exceeding maximum frequency (N32_PCLK2_MAX)"
#endif

/**
 * @brief   Timers 1, 2, 3, 4 clock (APB1 timers).
 */
#if (N32_APB1DIV == N32_APB1DIV_DIV1) || defined(__DOXYGEN__)
#define N32_TMRCLK1                   (N32_PCLK1 * 1)
#else
#define N32_TMRCLK1                   (N32_PCLK1 * 2)
#endif

/**
 * @brief   Timers 5, 6, 7, 8 clock (APB2 timers).
 */
#if (N32_APB2DIV == N32_APB2DIV_DIV1) || defined(__DOXYGEN__)
#define N32_TMRCLK2                   (N32_PCLK2 * 1)
#else
#define N32_TMRCLK2                   (N32_PCLK2 * 2)
#endif

/**
 * @brief   Flash settings.
 */
#if (N32_HCLK <= 48000000) || defined(__DOXYGEN__)
#define N32_FLASHBITS                 0x00000000U
#elif (N32_HCLK <= 96000000)
#define N32_FLASHBITS                 0x00000001U
#elif (N32_HCLK <= 144000000)
#define N32_FLASHBITS                 0x00000002U
#elif (N32_HCLK <= 192000000)
#define N32_FLASHBITS                 0x00000003U
#elif (N32_HCLK <= 240000000)
#define N32_FLASHBITS                 0x00000004U
#else
#error "N32_FLASHBITS: N32_HCLK too high"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/* Various helpers.*/
#include "nvic.h"
#include "cache.h"
#include "mpu_v7m.h"
#include "n32_isr.h"

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
  void n32_clock_init(void);
#ifdef __cplusplus
}
#endif
#endif /* !defined(_FROM_ASM_) */

#endif /* _HAL_LLD_H_ */

/** @} */
