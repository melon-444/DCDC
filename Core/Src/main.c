#include "ti_msp_dl_config.h"
#include "pid_delta.h"
#include "st7735_min.h"
#include "driver_ssd1306_font.h"
#include "driver_ssd1306_interface.h"

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

        break;
    default:
        break;
    }
}

/* ==================== main ==================== */
ssd1306_handle_t g_oled;

static void display_update(void)
{
    char buf[24];
    uint32_t vbat = gVsense_adcV * 1000;
    uint32_t ichg = gIsense_adcV * 1000;
    uint32_t duty_permille = ((uint32_t)g_duty_counts * 1000UL) / PWM_PERIOD;

    snprintf(buf, sizeof(buf), "%lu.%03luV   ", vbat / 1000UL, vbat % 1000UL);
    LCD_DrawString(42, 16, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%lumA    ", ichg);
    LCD_DrawString(42, 28, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%lu.%lu%%    ", duty_permille / 10UL, duty_permille % 10UL);
    LCD_DrawString(42, 40, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%s       ", state_to_string(g_state));
    LCD_DrawString(42, 52, buf, COLOR_WHITE, COLOR_BLACK);
}

int main(void)
{
    SYSCFG_DL_init();
    // OLED_DRIVER_INIT();

    DL_TimerA_startCounter(PWM_CHG_INST);
    DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, 320, DL_TIMER_CC_0_INDEX);

    uint16_t raw0 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_VBAT);
    uint16_t raw1 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_ICHG);

    gVsense_adcV = (float)raw0 * ADC_VREF_V / ADC_RESOLUTION;
    gIsense_adcV = (float)raw1 * ADC_VREF_V / ADC_RESOLUTION;
    gADC_ready = true;

    void LCD_Init(void);
    // LCD_DrawString( 0, 0, "hello world", 12, 0);

    while (1)
    {
        display_update();
        LCD_Update();

        triggerADC();

        if (isADCReady())
        {
            DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, PWM_PERIOD * buck_single_pid_delta_update(&buckpid, 0.5, getVsense(), getIsense()), DL_TIMER_CC_0_INDEX);
            clearADCReady();
            /* PID control entry point:
             *   duty = buck_pid_update(&buck, target_V, vsense, isense);
             *   uint16_t cmp = (uint16_t)(PWM_PERIOD * (1.0f - duty));
             *   DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, cmp,
             *                                    DL_TIMER_CC_0_INDEX);
             */
        }
    }
}
