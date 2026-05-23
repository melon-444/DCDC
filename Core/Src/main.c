#include "ti_msp_dl_config.h"
#include "pid_delta.h"
#include "driver_ssd1306.h"
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

int main(void)
{
    SYSCFG_DL_init();

    DL_TimerA_startCounter(PWM_CHG_INST);
    DL_TimerA_setCaptureCompareValue(PWM_CHG_INST, 320, DL_TIMER_CC_0_INDEX);

    uint16_t raw0 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_VBAT);
    uint16_t raw1 = DL_ADC12_getMemResult(ADC_SENSE_INST, ADC_SENSE_ADCMEM_ICHG);

    gVsense_adcV = (float)raw0 * ADC_VREF_V / ADC_RESOLUTION;
    gIsense_adcV = (float)raw1 * ADC_VREF_V / ADC_RESOLUTION;
    gADC_ready = true;

    ssd1306_interface_spi_init();
    // 1. 初始化 handle 并链接接口函数
    DRIVER_SSD1306_LINK_INIT(&g_oled, ssd1306_handle_t);
    DRIVER_SSD1306_LINK_IIC_INIT(&g_oled, ssd1306_interface_iic_init);
    DRIVER_SSD1306_LINK_IIC_DEINIT(&g_oled, ssd1306_interface_iic_deinit);
    DRIVER_SSD1306_LINK_IIC_WRITE(&g_oled, ssd1306_interface_iic_write);
    DRIVER_SSD1306_LINK_RESET_GPIO_INIT(&g_oled, ssd1306_interface_reset_gpio_init);
    DRIVER_SSD1306_LINK_RESET_GPIO_DEINIT(&g_oled, ssd1306_interface_reset_gpio_deinit);
    DRIVER_SSD1306_LINK_RESET_GPIO_WRITE(&g_oled, ssd1306_interface_reset_gpio_write);
    DRIVER_SSD1306_LINK_DELAY_MS(&g_oled, ssd1306_interface_delay_ms);
    DRIVER_SSD1306_LINK_DEBUG_PRINT(&g_oled, ssd1306_interface_debug_print);

    // 2. 初始化芯片
    ssd1306_set_interface(&g_oled, SSD1306_INTERFACE_IIC);
    ssd1306_set_addr_pin(&g_oled, SSD1306_ADDR_SA0_0); // 0x78
    ssd1306_init(&g_oled);
    ssd1306_clear(&g_oled);
    
    // 4. 画实心矩形（色块），参数：left, top, right, bottom, color
    ssd1306_gram_fill_rect(&g_oled, 0, 20, 30, 40, 1);

    // 3. 写文本（在 GRAM 中绘制，需要 update 才刷到屏幕）
    ssd1306_gram_write_string(&g_oled, 0, 0, "Hello", 5, 1, SSD1306_FONT_12);
    ssd1306_gram_update(&g_oled); // 刷新到屏幕


    while (1)
    {
        ssd1306_gram_update(&g_oled);
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
