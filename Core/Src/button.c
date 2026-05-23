#include "button.h"
#include "ti_msp_dl_config.h"

static volatile btn_event_t g_btn_event = BTN_EVENT_NONE;

void button_init(void)
{
    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_0_KEY_UP_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
         DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_0_KE_DOWN_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
         DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_GRP_0_KEY_MODE_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
         DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    uint32_t btn_pins = GPIO_GRP_0_KEY_START_PIN
                      | GPIO_GRP_0_KEY_UP_PIN
                      | GPIO_GRP_0_KE_DOWN_PIN
                      | GPIO_GRP_0_KEY_MODE_PIN;

    DL_GPIO_clearInterruptStatus(GPIO_GRP_0_PORT, btn_pins);
    DL_GPIO_enableInterrupt(GPIO_GRP_0_PORT, btn_pins);

    NVIC_EnableIRQ(GPIOB_INT_IRQn);
}

btn_event_t button_get_event(void)
{
    btn_event_t evt = g_btn_event;
    g_btn_event = BTN_EVENT_NONE;
    return evt;
}

void GROUP1_IRQHandler(void)
{
    uint32_t btn_pins = GPIO_GRP_0_KEY_START_PIN
                      | GPIO_GRP_0_KEY_UP_PIN
                      | GPIO_GRP_0_KE_DOWN_PIN
                      | GPIO_GRP_0_KEY_MODE_PIN;

    uint32_t status = DL_GPIO_getEnabledInterruptStatus(GPIO_GRP_0_PORT, btn_pins);
    if (status) {
        DL_GPIO_clearInterruptStatus(GPIO_GRP_0_PORT, status);

        if (status & GPIO_GRP_0_KEY_START_PIN) {
            g_btn_event = BTN_EVENT_START;
        }
        if (status & GPIO_GRP_0_KEY_UP_PIN) {
            g_btn_event = BTN_EVENT_UP;
        }
        if (status & GPIO_GRP_0_KE_DOWN_PIN) {
            g_btn_event = BTN_EVENT_DOWN;
        }
        if (status & GPIO_GRP_0_KEY_MODE_PIN) {
            g_btn_event = BTN_EVENT_MODE;
        }
    }
}
