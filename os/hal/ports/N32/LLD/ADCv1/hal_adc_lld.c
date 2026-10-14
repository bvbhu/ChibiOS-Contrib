/*
    ChibiOS - Copyright (C) 2024 bvbhu

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

/*
 * N32H49x ADCv1 low level driver.
 *
 * NOTES:
 *  - Only ADC1 is implemented (single conversion, interrupt driven, no
 *    DMA).  ADC2/ADC3 are deliberately not implemented yet: a clean
 *    #error is raised when they are requested in mcuconf.h.
 *  - The driver always configures the ADC at 12-bit resolution.  The
 *    QMK analog layer right-shifts raw samples to the desired
 *    resolution, so this is the correct behaviour for Vial keyboards.
 *  - The ADC clock is HSI (8MHz) / 8 = 1MHz by default (see
 *    N32_ADC_ADCHPRES_VALUE / N32_ADC_ADCM_PRES_VALUE macros below).
 */

#include "hal.h"

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if N32_ADC_USE_ADC1
/* ADC BUF (power/analog buffer) enable: undocumented register at
   0x4001004C, bit 13.  Required by the NSING SDK (ADC_Init). */
#define N32_ADCBUF_CTRL                 ((uint32_t)0x4001004CU)
#define N32_ADCBUF_EN_MASK              ((uint32_t)0x00002000U)

#ifndef N32_ADC_ADCHPRES_VALUE
#define N32_ADC_ADCHPRES_VALUE          RCC_CFG2_ADCHPRES_DIV16
#endif

#ifndef N32_ADC_ADCM_PRES_VALUE
#define N32_ADC_ADCM_PRES_VALUE         RCC_CFG2_ADC1MPRES_DIV8
#endif

#define N32_ADC_TIMEOUT                 ((uint32_t)1000000U)
#endif /* N32_ADC_USE_ADC1 */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if N32_ADC_USE_ADC1
/** @brief ADC1 driver identifier. */
ADCDriver ADCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#if N32_ADC_USE_ADC1
static void adc_lld_clock_enable(void) {
  RCC->AHB1PCLKEN |= RCC_AHB1PCLKEN_ADC1EN;
}

static void adc_lld_clock_disable(void) {
  RCC->AHB1PCLKEN &= ~RCC_AHB1PCLKEN_ADC1EN;
}

static void adc_lld_powerup_wait_ready(void) {
  uint32_t timeout = N32_ADC_TIMEOUT;

  ADC1->CTRL2 |= ADC_CTRL2_ON;
  while ((ADC1->STS & ADC_STS_RDY) == 0U) {
    if (--timeout == 0U) {
      break;
    }
  }
}

static void adc_lld_calibrate(void) {
  uint32_t timeout = N32_ADC_TIMEOUT;

  /* Reset calibration. */
  ADC1->CTRL2 |= ADC_CTRL2_RSTCAL;
  while ((ADC1->CTRL2 & ADC_CTRL2_RSTCAL) != 0U) {
    if (--timeout == 0U) {
      break;
    }
  }
  timeout = N32_ADC_TIMEOUT;

  /* Perform calibration.  Completion is signalled by the hardware
     clearing ENCAL. */
  ADC1->CTRL2 |= ADC_CTRL2_ENCAL;
  while ((ADC1->CTRL2 & ADC_CTRL2_ENCAL) != 0U) {
    if (--timeout == 0U) {
      break;
    }
  }
}

static void adc_lld_hw_init(void) {
  /* Plain memory-mapped write (the register is outside the CMSIS
     peripheral struct). */
  *((volatile uint32_t *)N32_ADCBUF_CTRL) |= N32_ADCBUF_EN_MASK;

  /* ADC power support: external MR + internal LDO. */
  ADC1->PCTRL   &= ~(ADC_PCTRL_EXTPEN | ADC_PCTRL_VREFLDOEN);
  ADC1->PCTRL   |=  (ADC_PCTRL_EXTPEN | ADC_PCTRL_VREFLDOEN);

  /* ADC clock: HSI 8MHz / 8 = 1MHz. */
  RCC->CFG2 &= ~(RCC_CFG2_ADCHPRES | RCC_CFG2_ADC1MPRES | RCC_CFG2_ADC1MSEL);
  RCC->CFG2 |= N32_ADC_ADCHPRES_VALUE | N32_ADC_ADCM_PRES_VALUE;

  /* Default configuration. */
  ADC1->CTRL1 = 0U;
  ADC1->CTRL2 = 0U;
  /* 12-bit resolution. */
  ADC1->CTRL3 = ADC_CTRL3_RES;
  ADC1->INTEN = 0U;
  ADC1->DSMUCHMASK = 0x0007FFFFU;
}
#endif /* N32_ADC_USE_ADC1 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   ADC driver initialization.
 * @note    This function is invoked once (during system init).
 */
void adc_lld_init(void) {

#if N32_ADC_USE_ADC1
  adcObjectInit(&ADCD1);
  ADCD1.adc = ADC1;

  adc_lld_clock_enable();
  adc_lld_hw_init();
  adc_lld_clock_disable();
#endif
}

/**
 * @brief   Configures and activates the ADC peripheral.
 */
void adc_lld_start(ADCDriver *adcp) {
#if N32_ADC_USE_ADC1
  (void)adcp;

  adc_lld_clock_enable();
  adc_lld_hw_init();

  /* Power up and wait for the ADC ready flag. */
  adc_lld_powerup_wait_ready();

  /* Perform the calibration sequence. */
  adc_lld_calibrate();
#endif
}

/**
 * @brief   Deactivates the ADC peripheral.
 */
void adc_lld_stop(ADCDriver *adcp) {
#if N32_ADC_USE_ADC1
  (void)adcp;

  ADC1->INTEN = 0U;
  nvicDisableVector(ADC1_2_IRQn);
  ADC1->CTRL2 = 0U;

  adc_lld_clock_disable();
#endif
}

/**
 * @brief   Starts an ADC conversion.
 * @note    Invoked in I-class context: no blocking allowed.
 */
void adc_lld_start_conversion(ADCDriver *adcp) {
#if N32_ADC_USE_ADC1
  const ADCConversionGroup *grpp = adcp->grpp;

  osalDbgAssert(grpp->num_channels == 1U,
                "N32 ADC driver: multi-channel conversions not supported");

  /* Configure the single channel sequence (rank 1) and sample time. */
  ADC1->CTRL1 = grpp->ctrl1;
  ADC1->SAMPT1 = grpp->smpt1;
  ADC1->SAMPT2 = grpp->smpt2;
  /* Sequence registers: SQx in RSEQ1..RSEQ3, length in RSEQ3[28:25]. */
  ADC1->RSEQ1 = grpp->rseq1;
  ADC1->RSEQ2 = grpp->rseq2;
  ADC1->RSEQ3 = (grpp->rseq3 & 0x001FFFFFU) |
                ADC_RSEQ3_LEN_N(grpp->num_channels);

  /* Clear pending end-of-conversion flag. */
  ADC1->STS = ADC_STS_ENDC;

  /* Enable the end-of-conversion interrupt. */
  ADC1->INTEN = ADC_INTEN_ENDCIEN;
  nvicEnableVector(ADC1_2_IRQn, N32_ADC_ADC1_IRQ_PRIORITY);

  /* Start the conversion with a software trigger.
     ON must stay set while a conversion runs. */
  ADC1->CTRL2 = (grpp->ctrl2 & ~ADC_CTRL2_ON) | ADC_CTRL2_ON |
                ADC_CTRL2_SWSTRRCH;
#endif
}

/**
 * @brief   Stops an ongoing ADC conversion.
 * @note    Invoked in I-class context.
 */
void adc_lld_stop_conversion(ADCDriver *adcp) {
#if N32_ADC_USE_ADC1
  (void)adcp;

  ADC1->INTEN = 0U;
  nvicDisableVector(ADC1_2_IRQn);
  ADC1->STS = ADC_STS_ENDC;
  ADC1->CTRL2 &= ~ADC_CTRL2_SWSTRRCH;
#endif
}

/**
 * @brief   ADC interrupt service routine handler.
 */
void adc_lld_serve_interrupt(ADCDriver *adcp) {
#if N32_ADC_USE_ADC1
  if ((ADC1->STS & ADC_STS_ENDC) != 0U) {
    uint32_t sample;

    /* Capture the conversion result. */
    sample = ADC1->DAT;
    ADC1->STS = ADC_STS_ENDC;

    /* Store the single sample into the driver buffer. */
    adcp->samples[0] = (adcsample_t)(sample & 0xFFFFU);

    /* Wake up the waiting thread. */
    _adc_isr_full_code(adcp);
  }
#endif
}

#if N32_ADC_USE_ADC1
OSAL_IRQ_HANDLER(N32_ADC1_2_HANDLER) {
  OSAL_IRQ_PROLOGUE();
  adc_lld_serve_interrupt(&ADCD1);
  OSAL_IRQ_EPILOGUE();
}
#endif

#endif /* HAL_USE_ADC */

/** @} */