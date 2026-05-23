/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_CHG */
#define PWM_CHG_INST                                                       TIMA0
#define PWM_CHG_INST_IRQHandler                                 TIMA0_IRQHandler
#define PWM_CHG_INST_INT_IRQN                                   (TIMA0_INT_IRQn)
#define PWM_CHG_INST_CLK_FREQ                                           32000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_CHG_C0_PORT                                               GPIOA
#define GPIO_PWM_CHG_C0_PIN                                       DL_GPIO_PIN_21
#define GPIO_PWM_CHG_C0_IOMUX                                    (IOMUX_PINCM46)
#define GPIO_PWM_CHG_C0_IOMUX_FUNC                   IOMUX_PINCM46_PF_TIMA0_CCP0
#define GPIO_PWM_CHG_C0_IDX                                  DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 0 */
#define GPIO_PWM_CHG_C0_CMPL_PORT                                          GPIOA
#define GPIO_PWM_CHG_C0_CMPL_PIN                                  DL_GPIO_PIN_22
#define GPIO_PWM_CHG_C0_CMPL_IOMUX                               (IOMUX_PINCM47)
#define GPIO_PWM_CHG_C0_CMPL_IOMUX_FUNC         IOMUX_PINCM47_PF_TIMA0_CCP0_CMPL





/* Defines for ADC_SENSE */
#define ADC_SENSE_INST                                                      ADC0
#define ADC_SENSE_INST_IRQHandler                                ADC0_IRQHandler
#define ADC_SENSE_INST_INT_IRQN                                  (ADC0_INT_IRQn)
#define ADC_SENSE_ADCMEM_VBAT                                 DL_ADC12_MEM_IDX_0
#define ADC_SENSE_ADCMEM_VBAT_REF                DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC_SENSE_ADCMEM_VBAT_REF_VOLTAGE_V                                     3.3
#define ADC_SENSE_ADCMEM_ICHG                                 DL_ADC12_MEM_IDX_1
#define ADC_SENSE_ADCMEM_ICHG_REF                DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC_SENSE_ADCMEM_ICHG_REF_VOLTAGE_V                                     3.3
#define GPIO_ADC_SENSE_C0_PORT                                             GPIOA
#define GPIO_ADC_SENSE_C0_PIN                                     DL_GPIO_PIN_27
#define GPIO_ADC_SENSE_C1_PORT                                             GPIOA
#define GPIO_ADC_SENSE_C1_PIN                                     DL_GPIO_PIN_26



/* Port definition for Pin Group GPIO_GRP_0 */
#define GPIO_GRP_0_PORT                                                  (GPIOB)

/* Defines for LCD_SCL: GPIOB.9 with pinCMx 26 on package pin 61 */
#define GPIO_GRP_0_LCD_SCL_PIN                                   (DL_GPIO_PIN_9)
#define GPIO_GRP_0_LCD_SCL_IOMUX                                 (IOMUX_PINCM26)
/* Defines for LCD_SDA: GPIOB.8 with pinCMx 25 on package pin 60 */
#define GPIO_GRP_0_LCD_SDA_PIN                                   (DL_GPIO_PIN_8)
#define GPIO_GRP_0_LCD_SDA_IOMUX                                 (IOMUX_PINCM25)
/* Defines for LCD_RES: GPIOB.14 with pinCMx 31 on package pin 2 */
#define GPIO_GRP_0_LCD_RES_PIN                                  (DL_GPIO_PIN_14)
#define GPIO_GRP_0_LCD_RES_IOMUX                                 (IOMUX_PINCM31)
/* Defines for LCD_DC: GPIOB.10 with pinCMx 27 on package pin 62 */
#define GPIO_GRP_0_LCD_DC_PIN                                   (DL_GPIO_PIN_10)
#define GPIO_GRP_0_LCD_DC_IOMUX                                  (IOMUX_PINCM27)
/* Defines for LCD_CS: GPIOB.11 with pinCMx 28 on package pin 63 */
#define GPIO_GRP_0_LCD_CS_PIN                                   (DL_GPIO_PIN_11)
#define GPIO_GRP_0_LCD_CS_IOMUX                                  (IOMUX_PINCM28)
/* Defines for LCD_BLK: GPIOB.26 with pinCMx 57 on package pin 28 */
#define GPIO_GRP_0_LCD_BLK_PIN                                  (DL_GPIO_PIN_26)
#define GPIO_GRP_0_LCD_BLK_IOMUX                                 (IOMUX_PINCM57)
/* Defines for KEY_START: GPIOB.21 with pinCMx 49 on package pin 20 */
#define GPIO_GRP_0_KEY_START_PIN                                (DL_GPIO_PIN_21)
#define GPIO_GRP_0_KEY_START_IOMUX                               (IOMUX_PINCM49)
/* Defines for KEY_UP: GPIOB.12 with pinCMx 29 on package pin 64 */
#define GPIO_GRP_0_KEY_UP_PIN                                   (DL_GPIO_PIN_12)
#define GPIO_GRP_0_KEY_UP_IOMUX                                  (IOMUX_PINCM29)
/* Defines for KE_DOWN: GPIOB.13 with pinCMx 30 on package pin 1 */
#define GPIO_GRP_0_KE_DOWN_PIN                                  (DL_GPIO_PIN_13)
#define GPIO_GRP_0_KE_DOWN_IOMUX                                 (IOMUX_PINCM30)
/* Defines for KEY_MODE: GPIOB.15 with pinCMx 32 on package pin 3 */
#define GPIO_GRP_0_KEY_MODE_PIN                                 (DL_GPIO_PIN_15)
#define GPIO_GRP_0_KEY_MODE_IOMUX                                (IOMUX_PINCM32)



/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_CHG_init(void);
void SYSCFG_DL_ADC_SENSE_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
