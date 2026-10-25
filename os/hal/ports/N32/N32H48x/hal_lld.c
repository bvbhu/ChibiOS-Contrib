/*
    ChibiOS - Copyright (C) 2026 bvbhu (vial-qmk-stg-n32h481 fork)

    N32H48x HAL low level driver source (NSING N32H481REL7K).

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
 * @file    N32H48x/hal_lld.c
 * @brief   N32H48x HAL subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   CMSIS system core clock variable.
 * @note    It is declared in system_N32H48x.h.
 */
uint32_t SystemCoreClock = N32_HCLK;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {

  /* Reset of all peripherals. */
  RCC->APB1PRST = 0xFFFFFFFF;
  RCC->APB1PRST = 0;
  RCC->APB2PRST = 0xFFFFFFFF;
  RCC->APB2PRST = 0;
  RCC->AHBPRST = 0xFFFFFFFF;
  RCC->AHBPRST = 0;

  /* PWR clocks enabled. */
  RCC->APB1PCLKEN |= RCC_APB1PCLKEN_PWREN;

  /* IRQ subsystem initialization.*/
  irqInit();
}

/**
 * @brief   N32 clocks and PLL initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function should be invoked just after the system reset.
 *
 * @special
 */
void n32_clock_init(void) {

#if !N32_NO_INIT
  uint32_t tmpregister;
  uint32_t tmpregister1;
  static const uint8_t plloutpres_table[10] = {0, 0, 1, 1, 2, 2, 2, 2, 3, 3};

  /* HSI setup, it enforces the reset situation in order to handle possible
     problems with JTAG probes and re-initializations.*/
  RCC->CTRL |= RCC_CTRL_HSIEN;              /* Make sure HSI is ON.           */
  while ((RCC->CTRL & RCC_CTRL_HSIRDF) == 0)
    ;                                       /* Wait until HSI is stable.      */

  /* HSI is selected as new source without touching the other fields in
     CFG. Clearing the register has to be postponed after HSI is the
     new source. */
  RCC->CFG &= ~RCC_CFG_SCLKSW;              /* Reset SCLKSW, selecting HSI.   */
  while ((RCC->CFG & RCC_CFG_SCLKSTS) != RCC_CFG_SCLKSTS_HSI)
    ;                                       /* Waits until HSI is selected.   */

  /* Resets all the other CFG fields. */
  RCC->CFG &= (uint32_t)0xC7F081FFU;

  /* Resets CTRL bits except HSIEN. */
  RCC->CTRL &= (uint32_t)0xFFFFFF2EU;
  while ((RCC->CTRL & RCC_CTRL_PLLRDF) != 0)
    ;                                       /* Waits until PLL is disabled.   */

  /* Resets PLL fields in CFG. */
  RCC->CFG &= (uint32_t)0xFFFFFE00U;

  /* Resets PLLINPRES, PLLOUTPRES and SCLKPLLSEL bits (like SDK SystemInit).
     LPFC2/LPFR/LPFC1 bits are restored by the configuration below. */
  RCC->PLLCTRL &= (uint32_t)0xFFC0FFFFU;

  /* Enable ICACHE and disable Prefetch Buffer. */
  FLASH->AC |= (uint32_t)(FLASH_AC_ICAHEN);
  FLASH->AC &= (uint32_t)(~FLASH_AC_PRFTBFEN);

  /* Flash wait states depending on clock.
     0: HCLK <= 40M, 1: <=80M, 2: <=120M, 3: <=160M, 4: <=200M, 5: <=240M
     (N32H481 user manual, Table for FLASH->AC LATENCY). */
  tmpregister = FLASH->AC;
  tmpregister &= (uint32_t)(~FLASH_AC_LATENCY);
  tmpregister |= (uint32_t)((N32_SYSCLK - 1) / 40000000);
  FLASH->AC = tmpregister;

  /* HCLK = SYSCLK. */
  RCC->CFG |= RCC_CFG_AHBPRES_DIV1;

  /* PCLK2/PCLK1 max 180MHz. */
  if (N32_SYSCLK > 180000000) {
    RCC->CFG |= ((uint32_t)RCC_CFG_APB2PRES_DIV2 | RCC_CFG_APB1PRES_DIV2);
  }
  else {
    RCC->CFG |= ((uint32_t)RCC_CFG_APB2PRES_DIV1 | RCC_CFG_APB1PRES_DIV1);
  }

#if (N32_SCLKSEL == N32_SCLKSEL_PLL)
  tmpregister = RCC->CFG;
  tmpregister1 = RCC->PLLCTRL;

  /* Clear bits. */
  tmpregister &= (uint32_t)(~(RCC_CFG_PLLSRC | RCC_CFG_PLLHSEPRES |
                              RCC_CFG_PLLHSIPRES | RCC_CFG_PLLMULFCT));
  tmpregister1 &= (uint32_t)(~(RCC_PLLCTRL_PLLINPRES | RCC_PLLCTRL_PLLOUTPRES |
                               RCC_PLLCTRL_LPFC2 | RCC_PLLCTRL_LPFR |
                               RCC_PLLCTRL_LPFC1));

  /* Set PLL source. */
#if N32_PLLSRC == N32_PLLSRC_HSE
  tmpregister |= RCC_CFG_PLLSRC;
#else
  tmpregister |= (uint32_t)0x00000000U;
#endif

  /* Set PLL multiply factor. */
  tmpregister |= N32_PLLMULFCT;

  /* Set LPFC2/LPFR/LPFC1 bits (N32H48x layout: LPFC2=bits12-14,
     LPFR=bits8-11, LPFC1=bits4-7). Note: no SCLKPLLSEL bits here. */
  if ((tmpregister & RCC_CFG_PLLMULFCT) <= RCC_CFG_PLLMULFCT24) {
    tmpregister1 |= (uint32_t)0x00006340;
  }
  else if ((tmpregister & RCC_CFG_PLLMULFCT) <= RCC_CFG_PLLMULFCT40) {
    tmpregister1 |= (uint32_t)0x00006740;
  }
  else {
    tmpregister1 |= (uint32_t)0x00006C40;
  }

  /* Set PLLINPRES bits. */
  tmpregister1 |= (uint32_t)((N32_PLLINPRES_VALUE - 1) << 16);
  /* Set PLLOUTPRES bits. */
  tmpregister1 |= (uint32_t)(plloutpres_table[N32_PLLOUTPRES_VALUE] << 18);

  RCC->CFG = tmpregister;
  RCC->PLLCTRL = tmpregister1;

  /* Enable PLL. */
  RCC->CTRL |= RCC_CTRL_PLLEN;

  /* Wait till PLL is ready. */
  while ((RCC->CTRL & RCC_CTRL_PLLRDF) == 0)
    ;

  /* Select PLL as system clock source. */
  tmpregister = RCC->CFG;
  tmpregister &= (uint32_t)(~(RCC_CFG_SCLKSW));
  tmpregister |= N32_SCLKSW_PLL;
  RCC->CFG = tmpregister;

  /* Wait till PLL is used as system clock source. */
  while ((RCC->CFG & RCC_CFG_SCLKSTS) != RCC_CFG_SCLKSTS_PLL)
    ;
#endif /* N32_SCLKSEL == N32_SCLKSEL_PLL */

  /* Default Enable PF Clock. */
  RCC->AHB1PCLKEN |= RCC_AHB1PCLKEN_IOPFEN;
  /* Default Configures the RNGCCLK divide 2 from sysclk. */
  RCC->CFG2 |= RCC_CFG2_RNGCPRES_DIV2;
#endif /* !N32_NO_INIT */
}

/** @} */
