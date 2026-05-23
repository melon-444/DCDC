/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gPWM_CHGBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_CHG_init();
    SYSCFG_DL_ADC_SENSE_init();
    SYSCFG_DL_SYSTICK_init();
    /* Ensure backup structures have no valid state */
	gPWM_CHGBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(PWM_CHG_INST, &gPWM_CHGBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(PWM_CHG_INST, &gPWM_CHGBackup, false);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(PWM_CHG_INST);
    DL_ADC12_reset(ADC_SENSE_INST);


    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(PWM_CHG_INST);
    DL_ADC12_enablePower(ADC_SENSE_INST);

    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_CHG_C0_IOMUX,GPIO_PWM_CHG_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_CHG_C0_PORT, GPIO_PWM_CHG_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_CHG_C0_CMPL_IOMUX,GPIO_PWM_CHG_C0_CMPL_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_CHG_C0_CMPL_PORT, GPIO_PWM_CHG_C0_CMPL_PIN);
    
	DL_GPIO_initPeripheralOutputFunction(
		 GPIO_PWM_CHG_C0_IOMUX, GPIO_PWM_CHG_C0_IOMUX_FUNC);
	DL_GPIO_initPeripheralOutputFunction(
		 GPIO_PWM_CHG_C0_CMPL_IOMUX, GPIO_PWM_CHG_C0_CMPL_IOMUX_FUNC);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_LCD_SCL_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_LCD_SDA_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_LCD_RES_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_LCD_DC_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_LCD_CS_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_GRP_0_LCD_BLK_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_0_KEY_START_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_0_KEY_UP_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_0_KE_DOWN_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_0_KEY_MODE_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_setPins(GPIO_GRP_0_PORT, GPIO_GRP_0_LCD_SCL_PIN |
		GPIO_GRP_0_LCD_SDA_PIN |
		GPIO_GRP_0_LCD_RES_PIN |
		GPIO_GRP_0_LCD_DC_PIN |
		GPIO_GRP_0_LCD_CS_PIN |
		GPIO_GRP_0_LCD_BLK_PIN);
    DL_GPIO_enableOutput(GPIO_GRP_0_PORT, GPIO_GRP_0_LCD_SCL_PIN |
		GPIO_GRP_0_LCD_SDA_PIN |
		GPIO_GRP_0_LCD_RES_PIN |
		GPIO_GRP_0_LCD_DC_PIN |
		GPIO_GRP_0_LCD_CS_PIN |
		GPIO_GRP_0_LCD_BLK_PIN);

}


SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    /* Set default configuration */
    DL_SYSCTL_disableHFXT();
    DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_1);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

}


/*
 * Timer clock configuration to be sourced by  / 1 (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_CHGClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gPWM_CHGConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 640,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_CHG_init(void) {

    DL_TimerA_setClockConfig(
        PWM_CHG_INST, (DL_TimerA_ClockConfig *) &gPWM_CHGClockConfig);

    DL_TimerA_initPWMMode(
        PWM_CHG_INST, (DL_TimerA_PWMConfig *) &gPWM_CHGConfig);

    DL_TimerA_setCaptureCompareOutCtl(PWM_CHG_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_DEAD_BAND,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_CHG_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, 640, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setDeadBand(PWM_CHG_INST, 16, 16, DL_TIMER_DEAD_BAND_MODE_0);
    DL_TimerA_enableClock(PWM_CHG_INST);


    DL_TimerA_enableInterrupt(PWM_CHG_INST , DL_TIMER_INTERRUPT_ZERO_EVENT);

    DL_TimerA_setCCPDirection(PWM_CHG_INST , DL_TIMER_CC0_OUTPUT );


}


/* ADC_SENSE Initialization */
static const DL_ADC12_ClockConfig gADC_SENSEClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_SYSOSC,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADC_SENSE_init(void)
{
    DL_ADC12_setClockConfig(ADC_SENSE_INST, (DL_ADC12_ClockConfig *) &gADC_SENSEClockConfig);

    DL_ADC12_initSeqSample(ADC_SENSE_INST,
        DL_ADC12_REPEAT_MODE_DISABLED, DL_ADC12_SAMPLING_SOURCE_AUTO, DL_ADC12_TRIG_SRC_SOFTWARE,
        DL_ADC12_SEQ_START_ADDR_00, DL_ADC12_SEQ_END_ADDR_01, DL_ADC12_SAMP_CONV_RES_12_BIT,
        DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADC_SENSE_INST, ADC_SENSE_ADCMEM_VBAT,
        DL_ADC12_INPUT_CHAN_0, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_configConversionMem(ADC_SENSE_INST, ADC_SENSE_ADCMEM_ICHG,
        DL_ADC12_INPUT_CHAN_1, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setSampleTime0(ADC_SENSE_INST,64000);
    DL_ADC12_setSampleTime1(ADC_SENSE_INST,64000);
    /* Enable ADC12 interrupt */
    DL_ADC12_clearInterruptStatus(ADC_SENSE_INST,(DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED));
    DL_ADC12_enableInterrupt(ADC_SENSE_INST,(DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED));
    DL_ADC12_enableConversions(ADC_SENSE_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_SYSTICK_init(void)
{
}

