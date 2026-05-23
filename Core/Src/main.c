#include "ti_msp_dl_config.h"
#include "pid_delta.h"
#include "button.h"
#include "st7735_min.h"
#include "driver_ssd1306_font.h"
#include "driver_ssd1306_interface.h"

static CascadedPID buckpid;

#define PWM_PERIOD (640U)
#define ADC_VREF_V (3.3f)
#define ADC_RESOLUTION (4095.0f)

volatile uint16_t gPWM_dutyCompare = 640;
volatile float gVsense_adcV;
volatile float gIsense_adcV;
volatile bool gADC_ready = false;
volatile bool queried_gADC = false;

typedef enum
{
    BMS_IDLE = 0,
    BMS_CHARGING,
    BMS_DONE,
    BMS_FAULT,
    BMS_PWM_TEST
} bms_state_t;

static const char* state_to_string(bms_state_t st)
{
    switch (st)
    {
    case BMS_IDLE:
        return "IDLE";
    case BMS_CHARGING:
        return "CHG";
    case BMS_DONE:
        return "DONE";
    case BMS_FAULT:
        return "FAULT";
    default:
        return "?";
    }
}

bms_state_t g_state = BMS_IDLE;


static volatile uint32_t adc_irq_count = 0;

/* ==================== Sensor API ==================== */
inline void triggerADC(void)
{
    DL_ADC12_startConversion(ADC_SENSE_INST);
}

#define VOLTAGE_K 0.16f;
float getVsense(void)
{
    return gVsense_adcV*2.037f/VOLTAGE_K;
}
#define CURRENT_K 2.2f;
float getIsense(void)
{
    return gIsense_adcV*1.675f/CURRENT_K;
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
void ADC_SENSE_INST_IRQHandler(void)
{
    
    switch (DL_ADC12_getPendingInterrupt(ADC_SENSE_INST))
    {
    case DL_ADC12_IIDX_MEM1_RESULT_LOADED:
    {
        adc_irq_count += 1;
        uint16_t raw0 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_VBAT);
        uint16_t raw1 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_ICHG);

        gVsense_adcV = (float)raw0 * ADC_VREF_V / ADC_RESOLUTION;
        gIsense_adcV = (float)raw1 * ADC_VREF_V / ADC_RESOLUTION;
        gADC_ready = true;
        if(!single_pid_should_stop(&buckpid))
            g_state=(BMS_CHARGING);

        DL_ADC12_clearInterruptStatus(
            ADC_SENSE_INST,
            DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
        break;
    }
    default:
    {   if(!single_pid_should_stop(&buckpid))
            g_state=(BMS_FAULT);
        break;
    }
    }
}

/* ==================== PWM ISR ==================== */
void TIMA0_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(PWM_CHG_INST))
    {
    case DL_TIMER_IIDX_ZERO:

        break;
    default:
        // DL_ADC12_startConversion(ADC_SENSE_INST);
        break;
    }
}

/* ==================== main ==================== */

static float g_duty_ratio = 0.0f;
static char buf[24];


static void display_update(void)
{
    
    uint32_t vbat = getVsense() * 1000;
    uint32_t ichg = getIsense() * 1000;
    uint32_t duty_permille = (uint32_t)(g_duty_ratio * 1000UL);
    uint32_t counting = adc_irq_count;

    snprintf(buf, sizeof(buf), "%lu.%03luV   ", (vbat / 1000UL), (vbat % 1000UL));
    LCD_DrawString(42, 16, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%umA    ", ichg);
    LCD_DrawString(42, 28, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%lu.%lu%%    ", (duty_permille / 10UL), (duty_permille % 10UL));
    LCD_DrawString(42, 40, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%s", state_to_string(g_state));
    LCD_DrawString(42, 52, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%u", counting);
    LCD_DrawString(82, 52, buf, COLOR_WHITE, COLOR_BLACK);
}

int main(void)
{
    SYSCFG_DL_init();

    DL_GPIO_initPeripheralAnalogFunction(IOMUX_PINCM44);
    DL_GPIO_initPeripheralAnalogFunction(IOMUX_PINCM45);
    

    button_init();
    
    NVIC_ClearPendingIRQ(ADC_SENSE_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(PWM_CHG_INST_INT_IRQN);
    NVIC_EnableIRQ(ADC_SENSE_INST_INT_IRQN);
    NVIC_EnableIRQ(PWM_CHG_INST_INT_IRQN);

    DL_ADC12_enableConversions(ADC_SENSE_INST);
    DL_ADC12_enablePower(ADC_SENSE_INST);

    DL_TimerA_startCounter(PWM_CHG_INST);
    DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, 320, DL_TIMER_CC_0_INDEX);



    LCD_Init();

    buck_single_pid_delta_init(&buckpid, 4.2f, 0.5f, 0.1f, 0.0001f);
    while (1){
        LCD_FillScreen(0x000000);
        LCD_DrawString(0, 0, "BMS BASIC", COLOR_WHITE, COLOR_BLACK);
        LCD_DrawString(0, 16, "VBAT:", COLOR_WHITE, COLOR_BLACK);
        LCD_DrawString(0, 28, "ICHG:", COLOR_WHITE, COLOR_BLACK);
        LCD_DrawString(0, 40, "DUTY:", COLOR_WHITE, COLOR_BLACK);
        LCD_DrawString(0, 52, "STAT:", COLOR_WHITE, COLOR_BLACK);
        display_update();
        LCD_Update();

        if (!DL_ADC12_isConversionStarted(ADC_SENSE_INST))
        {
            triggerADC();
        }

        if (isADCReady())
        {
            g_duty_ratio = buck_single_pid_delta_update(&buckpid,0.5, getVsense(), getIsense());

            DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, PWM_PERIOD * g_duty_ratio, DL_TIMER_CC_0_INDEX);
            clearADCReady();
            /* PID control entry point:
             *   duty = buck_pid_update(&buck, target_V, vsense, isense);
             *   uint16_t cmp = (uint16_t)(PWM_PERIOD * (1.0f - duty));
             *   DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, cmp,
             *                                    DL_TIMER_CC_0_INDEX);
             */
        }
        if(single_pid_should_stop(&buckpid)){
            g_state=(BMS_DONE);
            DL_TimerA_stopCounter(PWM_CHG_INST);
            break;
        }


    }
}
