/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio
    ChibiOS - Copyright (C) 2026 bvbhu (vial-qmk-stg-n32h481 fork)

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
 * @file    ADCv1/hal_adc_lld.h
 * @brief   N32 ADC subsystem low level driver header.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef HAL_ADC_LLD_H
#define HAL_ADC_LLD_H

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Available analog channels
 * @{
 */
#define ADC_CHANNEL_IN0           0  /**< @brief External analog input 0.    */
#define ADC_CHANNEL_IN1           1  /**< @brief External analog input 1.    */
#define ADC_CHANNEL_IN2           2  /**< @brief External analog input 2.    */
#define ADC_CHANNEL_IN3           3  /**< @brief External analog input 3.    */
#define ADC_CHANNEL_IN4           4  /**< @brief External analog input 4.    */
#define ADC_CHANNEL_IN5           5  /**< @brief External analog input 5.    */
#define ADC_CHANNEL_IN6           6  /**< @brief External analog input 6.    */
#define ADC_CHANNEL_IN7           7  /**< @brief External analog input 7.    */
#define ADC_CHANNEL_IN8           8  /**< @brief External analog input 8.    */
#define ADC_CHANNEL_IN9           9  /**< @brief External analog input 9.    */
#define ADC_CHANNEL_IN10          10 /**< @brief External analog input 10.   */
#define ADC_CHANNEL_IN11          11 /**< @brief External analog input 11.   */
#define ADC_CHANNEL_IN12          12 /**< @brief External analog input 12.   */
#define ADC_CHANNEL_IN13          13 /**< @brief External analog input 13.   */
#define ADC_CHANNEL_IN14          14 /**< @brief External analog input 14.   */
#define ADC_CHANNEL_IN15          15 /**< @brief External analog input 15.   */
#define ADC_CHANNEL_SENSOR        16 /**< @brief Internal temperature sensor.*/
#define ADC_CHANNEL_VREF          17 /**< @brief Internal reference.         */
/** @} */

/**
 * @name    Sampling rates
 * @details Values match the N32 SAMPTx register field encoding
 *          (4 bits per channel), see SDK N32H48x_adc.h
 *          ADC_SAMP_TIME_CYCLES_*.
 * @{
 */
#define ADC_SAMPLE_1P5            0  /**< @brief 1.5 cycles sampling time.   */
#define ADC_SAMPLE_2P5            1  /**< @brief 2.5 cycles sampling time.   */
#define ADC_SAMPLE_4P5            2  /**< @brief 4.5 cycles sampling time.   */
#define ADC_SAMPLE_7P5            3  /**< @brief 7.5 cycles sampling time.   */
#define ADC_SAMPLE_13P5           4  /**< @brief 13.5 cycles sampling time.  */
#define ADC_SAMPLE_19P5           5  /**< @brief 19.5 cycles sampling time.  */
#define ADC_SAMPLE_28P5           6  /**< @brief 28.5 cycles sampling time.  */
#define ADC_SAMPLE_41P5           7  /**< @brief 41.5 cycles sampling time.  */
#define ADC_SAMPLE_55P5           8  /**< @brief 55.5 cycles sampling time.  */
#define ADC_SAMPLE_61P5           9  /**< @brief 61.5 cycles sampling time.  */
#define ADC_SAMPLE_71P5           10 /**< @brief 71.5 cycles sampling time.  */
#define ADC_SAMPLE_181P5          11 /**< @brief 181.5 cycles sampling time. */
#define ADC_SAMPLE_239P5          12 /**< @brief 239.5 cycles sampling time. */
#define ADC_SAMPLE_601P5          13 /**< @brief 601.5 cycles sampling time. */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   ADC1 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(N32_ADC_USE_ADC1) || defined(__DOXYGEN__)
#define N32_ADC_USE_ADC1                    FALSE
#endif

/**
 * @brief   ADC1 interrupt priority level setting.
 */
#if !defined(N32_ADC_ADC1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define N32_ADC_ADC1_IRQ_PRIORITY           5
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if N32_ADC_USE_ADC1 && !N32_HAS_ADC1
#error "ADC1 not present in the selected device"
#endif

#if !N32_ADC_USE_ADC1 && !N32_ADC_USE_ADC2 && !N32_ADC_USE_ADC3
#error "ADC driver activated but no ADC peripheral assigned"
#endif

#if N32_ADC_USE_ADC2 || N32_ADC_USE_ADC3
#error "N32 driver: ADC2/ADC3 not implemented yet"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   ADC sample data type.
 */
typedef uint16_t adcsample_t;

/**
 * @brief   Channels number in a conversion group.
 */
typedef uint16_t adc_channels_num_t;

/**
 * @brief   Possible ADC failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  ADC_ERR_DMAFAILURE = 0                    /**< DMA operations failure.     */
} adcerror_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the ADC driver structure.
 */
#define adc_lld_driver_fields                                                \
  /* Pointer to the ADCx registers block.*/                                  \
  ADC_Module                *adc

/**
 * @brief   Low level fields of the ADC configuration structure.
 */
#define adc_lld_config_fields                                                \
  /* Dummy configuration, consign it to 0.*/                                 \
  uint32_t                  dummy

/**
 * @brief   Low level fields of the ADC configuration structure.
 * @details Sample times are packed register-wise exactly like the
 *          hardware layout: SAMPT1 holds channels 0..7, SAMPT2 holds
 *          channels 8..15 (and SAMPT3 channels 16..18 which are only
 *          used through ADC_CHANNEL_SENSOR/VREF). The group sequence is
 *          written into RSEQ1/2/3; the sequence length is enforced by
 *          the driver from @p num_channels.
 */
#define adc_lld_configuration_group_fields                                   \
  /* ADC CTRL1 register initialization data.                                 \
     NOTE: SCANMD and MULTMODE bits are passed through unmodified.*/         \
  uint32_t                  ctrl1;                                           \
  /* ADC CTRL2 register initialization data.                                 \
     NOTE: ADC_CTRL2_ON and ADC_CTRL2_SWSTRRCH are enforced inside the      \
     driver.*/                                                               \
  uint32_t                  ctrl2;                                           \
  /* ADC SAMPT1 register initialization data (channels 0..7).*/              \
  uint32_t                  smpt1;                                           \
  /* ADC SAMPT2 register initialization data (channels 8..15).*/             \
  uint32_t                  smpt2;                                           \
  /* ADC RSEQ1 register initialization data (sequence ranks 1..6).*/         \
  uint32_t                  rseq1;                                           \
  /* ADC RSEQ2 register initialization data (sequence ranks 7..12).*/        \
  uint32_t                  rseq2;                                           \
  /* ADC RSEQ3 register initialization data (sequence ranks 13..16).*/       \
  uint32_t                  rseq3

/**
 * @name    Sequences building helper macros
 * @{
 */
/**
 * @brief   Number of channels in a conversion sequence (writes RSEQ3.LEN).
 */
#define ADC_RSEQ3_LEN_N(n)      (((n) - 1) << 25)

#define ADC_RSEQ1_SQ1_N(n)      ((n) << 0)  /**< @brief 1st channel in seq.  */
#define ADC_RSEQ1_SQ2_N(n)      ((n) << 5)  /**< @brief 2nd channel in seq.  */
#define ADC_RSEQ1_SQ3_N(n)      ((n) << 10) /**< @brief 3rd channel in seq.  */
#define ADC_RSEQ1_SQ4_N(n)      ((n) << 15) /**< @brief 4th channel in seq.  */
#define ADC_RSEQ1_SQ5_N(n)      ((n) << 20) /**< @brief 5th channel in seq.  */
#define ADC_RSEQ1_SQ6_N(n)      ((n) << 25) /**< @brief 6th channel in seq.  */

#define ADC_RSEQ2_SQ7_N(n)      ((n) << 0)  /**< @brief 7th channel in seq.  */
#define ADC_RSEQ2_SQ8_N(n)      ((n) << 5)  /**< @brief 8th channel in seq.  */
#define ADC_RSEQ2_SQ9_N(n)      ((n) << 10) /**< @brief 9th channel in seq.  */
#define ADC_RSEQ2_SQ10_N(n)     ((n) << 15) /**< @brief 10th channel in seq. */
#define ADC_RSEQ2_SQ11_N(n)     ((n) << 20) /**< @brief 11th channel in seq. */
#define ADC_RSEQ2_SQ12_N(n)     ((n) << 25) /**< @brief 12th channel in seq. */

#define ADC_RSEQ3_SQ13_N(n)     ((n) << 0)  /**< @brief 13th channel in seq. */
#define ADC_RSEQ3_SQ14_N(n)     ((n) << 5)  /**< @brief 14th channel in seq. */
#define ADC_RSEQ3_SQ15_N(n)     ((n) << 10) /**< @brief 15th channel in seq. */
#define ADC_RSEQ3_SQ16_N(n)     ((n) << 15) /**< @brief 16th channel in seq. */
/** @} */

/**
 * @name    Sampling rate settings helper macros
 * @{
 */
#define ADC_SAMPT1_CSMP_AN0(n)  ((n) << 0)  /**< @brief AN0 sampling time.   */
#define ADC_SAMPT1_CSMP_AN1(n)  ((n) << 4)  /**< @brief AN1 sampling time.   */
#define ADC_SAMPT1_CSMP_AN2(n)  ((n) << 8)  /**< @brief AN2 sampling time.   */
#define ADC_SAMPT1_CSMP_AN3(n)  ((n) << 12) /**< @brief AN3 sampling time.   */
#define ADC_SAMPT1_CSMP_AN4(n)  ((n) << 16) /**< @brief AN4 sampling time.   */
#define ADC_SAMPT1_CSMP_AN5(n)  ((n) << 20) /**< @brief AN5 sampling time.   */
#define ADC_SAMPT1_CSMP_AN6(n)  ((n) << 24) /**< @brief AN6 sampling time.   */
#define ADC_SAMPT1_CSMP_AN7(n)  ((n) << 28) /**< @brief AN7 sampling time.   */

#define ADC_SAMPT2_CSMP_AN8(n)  ((n) << 0)  /**< @brief AN8 sampling time.   */
#define ADC_SAMPT2_CSMP_AN9(n)  ((n) << 4)  /**< @brief AN9 sampling time.   */
#define ADC_SAMPT2_CSMP_AN10(n) ((n) << 8)  /**< @brief AN10 sampling time.  */
#define ADC_SAMPT2_CSMP_AN11(n) ((n) << 12) /**< @brief AN11 sampling time.  */
#define ADC_SAMPT2_CSMP_AN12(n) ((n) << 16) /**< @brief AN12 sampling time.  */
#define ADC_SAMPT2_CSMP_AN13(n) ((n) << 20) /**< @brief AN13 sampling time.  */
#define ADC_SAMPT2_CSMP_AN14(n) ((n) << 24) /**< @brief AN14 sampling time.  */
#define ADC_SAMPT2_CSMP_AN15(n) ((n) << 28) /**< @brief AN15 sampling time.  */
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if N32_ADC_USE_ADC1 && !defined(__DOXYGEN__)
extern ADCDriver ADCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void adc_lld_init(void);
  void adc_lld_start(ADCDriver *adcp);
  void adc_lld_stop(ADCDriver *adcp);
  void adc_lld_start_conversion(ADCDriver *adcp);
  void adc_lld_stop_conversion(ADCDriver *adcp);
  void adc_lld_serve_interrupt(ADCDriver *adcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC */

#endif /* HAL_ADC_LLD_H */

/** @} */