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
 * @file    N32H49x/n32_isr.h
 * @brief   N32H49x ISR handler header.
 *
 * @addtogroup N32H49x_ISR
 * @{
 */

#ifndef N32_ISR_H
#define N32_ISR_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    ISRs suppressed in standard drivers
 * @{
 */
#define N32_GTIM1_SUPPRESS_ISR
#define N32_GTIM2_SUPPRESS_ISR
#define N32_GTIM3_SUPPRESS_ISR
#define N32_GTIM4_SUPPRESS_ISR
#define N32_GTIM5_SUPPRESS_ISR
#define N32_GTIM6_SUPPRESS_ISR
#define N32_GTIM7_SUPPRESS_ISR
#define N32_GTIM8_SUPPRESS_ISR
#define N32_GTIM9_SUPPRESS_ISR
#define N32_GTIM10_SUPPRESS_ISR
#define N32_ATIM1_SUPPRESS_ISR
#define N32_ATIM2_SUPPRESS_ISR
#define N32_ATIM3_SUPPRESS_ISR
#define N32_BTIM1_SUPPRESS_ISR
#define N32_BTIM2_SUPPRESS_ISR
/** @} */

/**
 * @name    N32H49x ISR names
 * @brief   IRQn to vector slot mapping (VectorXX = 0x40 + IRQn * 4).
 * @{
 */
#define N32_WWDG_HANDLER                 Vector40
#define N32_PVD_HANDLER                  Vector44
#define N32_RTC_TAMPER_HANDLER           Vector48
#define N32_RTC_WKUP_HANDLER             Vector4C
#define N32_FLASH_HANDLER                Vector50
#define N32_RCC_HANDLER                  Vector54
#define N32_EXINT0_HANDLER               Vector58
#define N32_EXINT1_HANDLER               Vector5C
#define N32_EXINT2_HANDLER               Vector60
#define N32_EXINT3_HANDLER               Vector64
#define N32_EXINT4_HANDLER               Vector68
#define N32_DMA1_CH1_HANDLER             Vector6C
#define N32_DMA1_CH2_HANDLER             Vector70
#define N32_DMA1_CH3_HANDLER             Vector74
#define N32_DMA1_CH4_HANDLER             Vector78
#define N32_DMA1_CH5_HANDLER             Vector7C
#define N32_DMA1_CH6_HANDLER             Vector80
#define N32_DMA1_CH7_HANDLER             Vector84
#define N32_ADC1_2_HANDLER               Vector88
#define N32_USB_FS_HP_HANDLER            Vector8C
#define N32_USB_FS_LP_HANDLER            Vector90
#define N32_FDCAN1_INT0_HANDLER          Vector94
#define N32_FDCAN1_INT1_HANDLER          Vector98
#define N32_EXINT5_9_HANDLER             Vector9C
#define N32_ATIM1_BRK_HANDLER            VectorA0
#define N32_ATIM1_UP_HANDLER             VectorA4
#define N32_ATIM1_TRG_COM_HANDLER        VectorA8
#define N32_ATIM1_CC_HANDLER             VectorAC
#define N32_GTIM1_HANDLER                VectorB0
#define N32_GTIM2_HANDLER                VectorB4
#define N32_GTIM3_HANDLER                VectorB8
#define N32_I2C1_EV_HANDLER              VectorBC
#define N32_I2C1_ER_HANDLER              VectorC0
#define N32_I2C2_EV_HANDLER              VectorC4
#define N32_I2C2_ER_HANDLER              VectorC8
#define N32_SPI1_HANDLER                 VectorCC
#define N32_SPI2_I2S2_HANDLER            VectorD0
#define N32_USART1_HANDLER               VectorD4
#define N32_USART2_HANDLER               VectorD8
#define N32_USART3_HANDLER               VectorDC
#define N32_EXINT10_15_HANDLER           VectorE0
#define N32_RTCALARM_HANDLER             VectorE4
#define N32_USB_FS_WKUP_HANDLER          VectorE8
#define N32_ATIM2_BRK_HANDLER            VectorEC
#define N32_ATIM2_UP_HANDLER             VectorF0
#define N32_ATIM2_TRG_COM_HANDLER        VectorF4
#define N32_ATIM2_CC_HANDLER             VectorF8
#define N32_ADC3_HANDLER                 VectorFC
#define N32_UCDR_HANDLER                 Vector100
#define N32_SDIO_HANDLER                 Vector104
#define N32_GTIM4_HANDLER                Vector108
#define N32_SPI3_I2S3_HANDLER            Vector10C
#define N32_USART4_HANDLER               Vector110
#define N32_UART5_HANDLER                Vector114
#define N32_BTIM1_HANDLER                Vector118
#define N32_BTIM2_HANDLER                Vector11C
#define N32_DMA2_CH1_HANDLER             Vector120
#define N32_DMA2_CH2_HANDLER             Vector124
#define N32_DMA2_CH3_HANDLER             Vector128
#define N32_DMA2_CH4_HANDLER             Vector12C
#define N32_DMA2_CH5_HANDLER             Vector130
#define N32_SRAM_ERR_HANDLER             Vector134
#define N32_USB_HS_HANDLER               Vector138
#define N32_FDCAN2_INT0_HANDLER          Vector13C
#define N32_FDCAN2_INT1_HANDLER          Vector140
#define N32_FDCAN3_INT0_HANDLER          Vector144
#define N32_FDCAN3_INT1_HANDLER          Vector148
#define N32_XSPI_HANDLER                 Vector14C
#define N32_DMA2_CH6_HANDLER             Vector150
#define N32_DMA2_CH7_HANDLER             Vector154
#define N32_I2C3_EV_HANDLER              Vector158
#define N32_I2C3_ER_HANDLER              Vector15C
#define N32_I2C4_EV_HANDLER              Vector160
#define N32_I2C4_ER_HANDLER              Vector164
#define N32_UART6_HANDLER                Vector168
#define N32_UART7_HANDLER                Vector16C
#define N32_DMA1_CH8_HANDLER             Vector170
#define N32_DMA2_CH8_HANDLER             Vector174
#define N32_DVP_HANDLER                  Vector178
#define N32_SAC_HANDLER                  Vector17C
#define N32_SMPU_HANDLER                 Vector180
#define N32_FEMC_HANDLER                 Vector184
#define N32_USB_HS_EPx_OUT_HANDLER       Vector188
#define N32_USB_HS_EPx_IN_HANDLER        Vector18C
#define N32_USB_HS_WKUP_HANDLER          Vector190
#define N32_UART8_HANDLER                Vector194
#define N32_SPI4_HANDLER                 Vector198
#define N32_ETH_HANDLER                  Vector19C
#define N32_ETH_WKUP_HANDLER             Vector1A0
#define N32_GTIM5_HANDLER                Vector1A4
#define N32_DSMU_FLT0_HANDLER            Vector1A8
#define N32_DSMU_FLT1_HANDLER            Vector1AC
#define N32_DSMU_FLT2_HANDLER            Vector1B0
#define N32_DSMU_FLT3_HANDLER            Vector1B4
#define N32_GTIM7_HANDLER                Vector1B8
#define N32_GTIM8_HANDLER                Vector1BC
#define N32_GTIM9_HANDLER                Vector1C0
#define N32_GTIM10_HANDLER               Vector1C4
#define N32_DAC1_2_UNDERFLOW_HANDLER     Vector1C8
#define N32_ATIM3_BRK_HANDLER            Vector1CC
#define N32_ATIM3_UP_HANDLER             Vector1D0
#define N32_ATIM3_TRG_COM_HANDLER        Vector1D4
#define N32_ATIM3_CC_HANDLER             Vector1D8
#define N32_GTIM6_HANDLER                Vector1DC
#define N32_LPTIM1_WKUP_HANDLER          Vector1E0
#define N32_LPTIM2_WKUP_HANDLER          Vector1E4
#define N32_CORDIC_HANDLER               Vector1EC
#define N32_SPI5_HANDLER                 Vector1F0
#define N32_SPI6_HANDLER                 Vector1F4
#define N32_USART9_HANDLER               Vector1F8
#define N32_UART10_HANDLER               Vector1FC
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void irqInit(void);
  void irqDeinit(void);
#ifdef __cplusplus
}
#endif

#endif /* N32_ISR_H */

/** @} */
