#include "ti_msp_dl_config.h"
#include "pid_delta.h"

BuckCascadedPID buckpid;

#define PWM_PERIOD (640U)
#define ADC_VREF_V (3.3f)
#define ADC_RESOLUTION (4096.0f)

volatile uint16_t gPWM_dutyCompare = 640;
volatile float gVsense_adcV;
volatile float gIsense_adcV;
volatile bool gADC_ready = false;
/* ==================== Sensor API ==================== */
inline void triggerADC(void)
{
    DL_ADC12_startConversion(ADC_SENSE_INST);
}

float getVsense(void)
{
    return gVsense_adcV;
}

float getIsense(void)
{
    return gIsense_adcV;
}

bool isADCReady(void)
{
    return gADC_ready;
}

void clearADCReady(void)
{
    gADC_ready = false;
}
/* ==================== ADC ISR ==================== */
void ADC0_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC_SENSE_INST))
    {
    case DL_ADC12_IIDX_MEM1_RESULT_LOADED:
    {
        uint16_t raw0 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_VBAT);
        uint16_t raw1 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_ICHG);

        gVsense_adcV = (float)raw0 * ADC_VREF_V / ADC_RESOLUTION;
        gIsense_adcV = (float)raw1 * ADC_VREF_V / ADC_RESOLUTION;
        gADC_ready = true;
        break;
    }
    default:
        break;
    }
}

/* ==================== PWM ISR ==================== */
void TIMA0_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(PWM_CHG_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        DL_ADC12_startConversion(ADC_SENSE_INST);
        if (isADCReady())
        {
            DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, PWM_PERIOD*buck_single_pid_delta_update(&buckpid, 0.5, getVsense(), getIsense()), DL_TIMER_CC_0_INDEX);
            clearADCReady();
            /* PID control entry point:
             *   duty = buck_pid_update(&buck, target_V, vsense, isense);
             *   uint16_t cmp = (uint16_t)(PWM_PERIOD * (1.0f - duty));
             *   DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, cmp,
             *                                    DL_TIMER_CC_0_INDEX);
             */
        }

        break;
    default:
        break;
    }
}



/* ==================== main ==================== */
int main(void)
{
    SYSCFG_DL_init();

    DL_TimerA_startCounter(PWM_CHG_INST);
    DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, 640, DL_TIMER_CC_0_INDEX);

    while (1)
    {
    }
}
