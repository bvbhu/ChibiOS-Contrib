/*
    ChibiOS - Copyright (C) 2026 bvbhu (vial-qmk-stg-n32h481 fork)

    N32 PAL low level driver code (NSING N32H481REL7K).

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
 * @file    GPIOv1/hal_pal_lld.c
 * @brief   N32 PAL low level driver code.
 *
 * @addtogroup PAL
 * @{
 */

#include "hal.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define N32_GPIO_EN_MASK                                                \
  ((N32_HAS_GPIOA ? RCC_AHB1PCLKEN_IOPAEN : 0) |                        \
   (N32_HAS_GPIOB ? RCC_AHB1PCLKEN_IOPBEN : 0) |                        \
   (N32_HAS_GPIOC ? RCC_AHB1PCLKEN_IOPCEN : 0) |                        \
   (N32_HAS_GPIOD ? RCC_AHB1PCLKEN_IOPDEN : 0) |                        \
   (N32_HAS_GPIOE ? RCC_AHB1PCLKEN_IOPEEN : 0) |                        \
   (N32_HAS_GPIOF ? RCC_AHB1PCLKEN_IOPFEN : 0) |                        \
   (N32_HAS_GPIOG ? RCC_AHB1PCLKEN_IOPGEN : 0) |                        \
   (N32_HAS_GPIOH ? RCC_AHB1PCLKEN_IOPHEN : 0))

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if (PAL_USE_WAIT == TRUE) || (PAL_USE_CALLBACKS == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Event records for the 16 GPIO EXTI channels.
 */
palevent_t _pal_events[16];
#endif

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
 * @brief   PAL driver initialization.
 *
 * @param[in] config    PAL configuration
 *
 * @notapi
 */
void _pal_lld_init(const PALConfig *config) {

#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
  unsigned i;

  for (i = 0; i < 16; i++) {
    _pal_init_event(i);
  }
#endif
  /* Enable GPIO clocks.*/
  RCC->AHB1PCLKEN |= N32_GPIO_EN_MASK;

  /* Initial GPIO setup.*/
  GPIOA->POD     = config->PAData.pod;
  GPIOA->PMODE   = config->PAData.pmode;
  GPIOA->PUPD    = config->PAData.pupd;
  GPIOA->POTYPE  = config->PAData.potype;
  GPIOA->DS      = config->PAData.ds;
  GPIOB->POD     = config->PBData.pod;
  GPIOB->PMODE   = config->PBData.pmode;
  GPIOB->PUPD    = config->PBData.pupd;
  GPIOB->POTYPE  = config->PBData.potype;
  GPIOB->DS      = config->PBData.ds;
#if N32_HAS_GPIOC || defined(__DOXYGEN__)
  GPIOC->POD     = config->PCData.pod;
  GPIOC->PMODE   = config->PCData.pmode;
  GPIOC->PUPD    = config->PCData.pupd;
  GPIOC->POTYPE  = config->PCData.potype;
  GPIOC->DS      = config->PCData.ds;
#endif
  GPIOD->POD     = config->PDData.pod;
  GPIOD->PMODE   = config->PDData.pmode;
  GPIOD->PUPD    = config->PDData.pupd;
  GPIOD->POTYPE  = config->PDData.potype;
  GPIOD->DS      = config->PDData.ds;
#if N32_HAS_GPIOE || defined(__DOXYGEN__)
  GPIOE->POD     = config->PEData.pod;
  GPIOE->PMODE   = config->PEData.pmode;
  GPIOE->PUPD    = config->PEData.pupd;
  GPIOE->POTYPE  = config->PEData.potype;
  GPIOE->DS      = config->PEData.ds;
#endif
#if N32_HAS_GPIOF || defined(__DOXYGEN__)
  GPIOF->POD     = config->PFData.pod;
  GPIOF->PMODE   = config->PFData.pmode;
  GPIOF->PUPD    = config->PFData.pupd;
  GPIOF->POTYPE  = config->PFData.potype;
  GPIOF->DS      = config->PFData.ds;
#endif
#if N32_HAS_GPIOG || defined(__DOXYGEN__)
  GPIOG->POD     = config->PGData.pod;
  GPIOG->PMODE   = config->PGData.pmode;
  GPIOG->PUPD    = config->PGData.pupd;
  GPIOG->POTYPE  = config->PGData.potype;
  GPIOG->DS      = config->PGData.ds;
#endif
#if N32_HAS_GPIOH || defined(__DOXYGEN__)
  GPIOH->POD     = config->PHData.pod;
  GPIOH->PMODE   = config->PHData.pmode;
  GPIOH->PUPD    = config->PHData.pupd;
  GPIOH->POTYPE  = config->PHData.potype;
  GPIOH->DS      = config->PHData.ds;
#endif
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    @p PAL_MODE_UNCONNECTED is implemented as analog at minimum
 *          speed.
 *
 * @param[in] port      the port identifier
 * @param[in] mask      the group mask
 * @param[in] mode      the mode
 *
 * @notapi
 */
void _pal_lld_setgroupmode(ioportid_t port,
                           ioportmask_t mask,
                           iomode_t mode) {

  uint32_t pmode   = (mode & PAL_N32_MODE_MASK) >> 0;
  uint32_t potype  = (mode & PAL_N32_OTYPE_MASK) >> 2;
  uint32_t ds      = (mode & PAL_N32_OSPEED_MASK) >> 3;
  uint32_t pupd    = (mode & PAL_N32_PUPDR_MASK) >> 5;
  uint32_t altr    = (mode & PAL_N32_ALTERNATE_MASK) >> 7;
  uint32_t bit     = 0;
  while (true) {
    if ((mask & 1) != 0) {
      uint32_t altrmask, m1, m2, m4;

      altrmask = altr << ((bit & 7) * 4);
      m1 = 1 << bit;
      m2 = 3 << (bit * 2);
      m4 = 15 << ((bit & 7) * 4);
      port->POTYPE  = (port->POTYPE & ~m1) | potype;
      port->DS      = (port->DS & ~m2) | ds;
      port->PUPD    = (port->PUPD & ~m2) | pupd;
      if ((mode & PAL_N32_MODE_MASK) == PAL_N32_MODE_ALTERNATE) {
        /* If going in alternate mode then the alternate number is set
           before switching mode in order to avoid glitches.*/
        port->AFSEL[bit / 8] = (port->AFSEL[bit / 8] & ~m4) | altrmask;
        port->PMODE = (port->PMODE & ~m2) | pmode;
      }
      else {
        /* If going into a non-alternate mode then the mode is switched
           before setting the alternate mode in order to avoid glitches.*/
        port->PMODE = (port->PMODE & ~m2) | pmode;
        port->AFSEL[bit / 8] = (port->AFSEL[bit / 8] & ~m4) | altrmask;
      }
    }
    mask >>= 1;
    if (!mask)
      return;
    potype <<= 1;
    ds <<= 2;
    pupd <<= 2;
    pmode <<= 2;
    bit++;
  }
}

/**
 * @brief   Pad event enable.
 * @note    Not supported on N32.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      pad event mode
 *
 * @notapi
 */
void _pal_lld_enablepadevent(ioportid_t port,
                             iopadid_t pad,
                             ioeventmode_t mode) {
  (void)port;
  (void)pad;
  (void)mode;
}

/**
 * @brief   Pad event disable.
 * @note    Not supported on N32.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
void _pal_lld_disablepadevent(ioportid_t port, iopadid_t pad) {
  (void)port;
  (void)pad;
}

/**
 * @brief   Pad event enable check.
 * @note    Not supported on N32.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
bool _pal_lld_ispadeventenabled(ioportid_t port, iopadid_t pad) {
  (void)port;
  (void)pad;
  return false;
}

#endif /* HAL_USE_PAL */

/** @} */
