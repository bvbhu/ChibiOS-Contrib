/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio
    ChibiOS - Copyright (C) 2023..2024 HorrorTroll
    ChibiOS - Copyright (C) 2023..2024 Zhaqian
    ChibiOS - Copyright (C) 2026 bvbhu (N32H49x port for vial-qmk-stg-n32h481)

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
 * @file    N32H49x/n32_isr.c
 * @brief   N32H49x ISR handler code.
 *
 * @addtogroup N32H49x_ISR
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
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
 * @brief   Enables IRQ sources.
 * @details Drivers requiring IRQ sources initialize them in their own
 *          init code, so this function is a placeholder.
 *
 * @notapi
 */
void irqInit(void) {
}

/**
 * @brief   Disables IRQ sources.
 *
 * @notapi
 */
void irqDeinit(void) {
}

/** @} */
