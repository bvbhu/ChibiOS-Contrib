/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio
    ChibiOS - Copyright (C) 2023..2024 HorrorTroll
    ChibiOS - Copyright (C) 2023..2024 Zhaqian
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
 * @file    N32H48x/n32_registry.h
 * @brief   N32H48x capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef N32_REGISTRY_H
#define N32_REGISTRY_H

#if defined(N32H481RE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    N32H481RE capabilities
 * @{
 */
/* GPIO attributes.*/
#if !defined(N32_HAS_GPIOA) || defined(__DOXYGEN__)
#define N32_HAS_GPIOA                     TRUE
#endif
#if !defined(N32_HAS_GPIOB) || defined(__DOXYGEN__)
#define N32_HAS_GPIOB                     TRUE
#endif
#if !defined(N32_HAS_GPIOC) || defined(__DOXYGEN__)
#define N32_HAS_GPIOC                     TRUE
#endif
#if !defined(N32_HAS_GPIOD) || defined(__DOXYGEN__)
#define N32_HAS_GPIOD                     TRUE
#endif
#if !defined(N32_HAS_GPIOE) || defined(__DOXYGEN__)
#define N32_HAS_GPIOE                     TRUE
#endif
#if !defined(N32_HAS_GPIOF) || defined(__DOXYGEN__)
#define N32_HAS_GPIOF                     TRUE
#endif
#if !defined(N32_HAS_GPIOG) || defined(__DOXYGEN__)
#define N32_HAS_GPIOG                     TRUE
#endif
#if !defined(N32_HAS_GPIOH) || defined(__DOXYGEN__)
#define N32_HAS_GPIOH                     TRUE
#endif

/* DMA attributes.*/
#define N32_HAS_DMA1                      TRUE
#define N32_HAS_DMA2                      TRUE
#define N32_DMA1_NUM_CHANNELS             8
#define N32_DMA2_NUM_CHANNELS             8

/* ADC attributes. Official datasheet: 4x 12bit 4.7Msps ADC
   (ADC1=14ch, ADC2=16ch, ADC3=19ch, ADC4=18ch, 47 external channels). */
#define N32_HAS_ADC1                      TRUE
#define N32_HAS_ADC2                      TRUE
#define N32_HAS_ADC3                      TRUE
#define N32_HAS_ADC4                      TRUE

/* USART/UART attributes. Official: 3x USART (USART1/2/3) + 4x UART. */
#define N32_HAS_USART1                    TRUE
#define N32_HAS_USART2                    TRUE
#define N32_HAS_USART3                    TRUE
#define N32_HAS_UART4                     TRUE
#define N32_HAS_UART5                     TRUE
#define N32_HAS_UART6                     TRUE
#define N32_HAS_UART7                     TRUE
#define N32_HAS_UART8                     TRUE

/* I2C attributes.*/
#define N32_HAS_I2C1                      TRUE
#define N32_HAS_I2C2                      TRUE
#define N32_HAS_I2C3                      TRUE
#define N32_HAS_I2C4                      TRUE

/* SPI attributes.*/
#define N32_HAS_SPI1                      TRUE
#define N32_HAS_SPI2                      TRUE
#define N32_HAS_SPI3                      TRUE
#define N32_HAS_SPI4                      TRUE
#define N32_HAS_SPI5                      TRUE
#define N32_HAS_SPI6                      TRUE

/* Timers attributes.*/
#define N32_HAS_ATIM1                     TRUE
#define N32_HAS_ATIM2                     TRUE
#define N32_HAS_ATIM3                     TRUE
#define N32_HAS_GTIM1                     TRUE
#define N32_HAS_GTIM2                     TRUE
#define N32_HAS_GTIM3                     TRUE
#define N32_HAS_GTIM4                     TRUE
#define N32_HAS_GTIM5                     TRUE
#define N32_HAS_GTIM6                     TRUE
#define N32_HAS_GTIM7                     TRUE
#define N32_HAS_GTIM8                     TRUE
#define N32_HAS_GTIM9                     TRUE
#define N32_HAS_GTIM10                    TRUE
#define N32_HAS_BTIM1                     TRUE
#define N32_HAS_BTIM2                     TRUE

/* USB attributes. N32H481 official datasheet (V1.2.0): only USB HS DualRole,
   no standalone USB FS Device, hence USBFS is FALSE. */
#define N32_HAS_USBHS                     TRUE
#define N32_USBHS_ENDPOINTS               6
#define N32_USBHS_FIFO_MEM_SIZE           1280
#define N32_HAS_USBFS                     FALSE

/* FDCAN attributes. N32H481 official datasheet (V1.2.0): no CAN-FD. */
#define N32_HAS_FDCAN1                    FALSE
#define N32_HAS_FDCAN2                    FALSE
#define N32_HAS_FDCAN3                    FALSE

/* EXINT attributes.*/
#define N32_EXINT_NUM_LINES               24
#define N32_EXINT_INTEN_MASK              0x00000000U

/* FLASH attributes (N32H481RE: 512K flash, sector based).*/
#define N32_FLASH_NUMBER_OF_BANKS         1
#define N32_FLASH_SECTOR_SIZE             2048U

#if !defined(N32_FLASH_SECTORS_PER_BANK) || defined(__DOXYGEN__)
#define N32_FLASH_SECTORS_PER_BANK        256 /* Maximum, can be redefined.*/
#endif

/* RTC attributes.*/
#define N32_HAS_RTC                       TRUE

#endif /* defined(N32H481RE) */

#endif /* N32_REGISTRY_H */

/** @} */
