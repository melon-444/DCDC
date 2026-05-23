#include "driver_ssd1306_interface.h"
#include "ti_msp_dl_config.h"
#include <stdarg.h>

#define IIC_PORT             GPIO_GRP_0_PORT
#define IIC_SCL_MASK         GPIO_GRP_0_LCD_SCL_PIN
#define IIC_SDA_MASK         GPIO_GRP_0_LCD_SDA_PIN
#define IIC_RESET_MASK       GPIO_GRP_0_LCD_RES_PIN

#define SDA_IN()             (IIC_PORT->DOE31_0 &= ~IIC_SDA_MASK)
#define SDA_OUT()            (IIC_PORT->DOE31_0 |= IIC_SDA_MASK)
#define IIC_SCL_H()          DL_GPIO_writePinsVal(IIC_PORT, IIC_SCL_MASK, IIC_SCL_MASK)
#define IIC_SCL_L()          DL_GPIO_writePinsVal(IIC_PORT, IIC_SCL_MASK, 0)
#define IIC_SDA_H()          DL_GPIO_writePinsVal(IIC_PORT, IIC_SDA_MASK, IIC_SDA_MASK)
#define IIC_SDA_L()          DL_GPIO_writePinsVal(IIC_PORT, IIC_SDA_MASK, 0)
#define READ_SDA()           (DL_GPIO_readPins(IIC_PORT, IIC_SDA_MASK) != 0)

static void iic_delay_us(uint32_t us)
{
    uint32_t cycles = us * (CPUCLK_FREQ / 1000000) / 4;
    for (volatile uint32_t i = 0; i < cycles; i++) {}
}

static void a_iic_start(void)
{
    SDA_OUT();
    IIC_SDA_H();
    IIC_SCL_H();
    iic_delay_us(4);
    IIC_SDA_L();
    iic_delay_us(4);
    IIC_SCL_L();
}

static void a_iic_stop(void)
{
    SDA_OUT();
    IIC_SCL_L();
    IIC_SDA_L();
    iic_delay_us(4);
    IIC_SCL_H();
    iic_delay_us(4);
    IIC_SDA_H();
    iic_delay_us(4);
}

static uint8_t a_iic_wait_ack(void)
{
    uint16_t uc_err_time = 0;

    SDA_IN();
    IIC_SDA_H();
    iic_delay_us(1);
    IIC_SCL_H();
    iic_delay_us(1);
    while (READ_SDA() != 0)
    {
        uc_err_time++;
        if (uc_err_time > 250)
        {
            a_iic_stop();
            return 1;
        }
    }
    IIC_SCL_L();

    return 0;
}

static void a_iic_send_byte(uint8_t txd)
{
    uint8_t t;

    SDA_OUT();
    IIC_SCL_L();
    for (t = 0; t < 8; t++)
    {
        if (txd & 0x80)
        {
            IIC_SDA_H();
        }
        else
        {
            IIC_SDA_L();
        }
        txd <<= 1;
        iic_delay_us(2);
        IIC_SCL_H();
        iic_delay_us(2);
        IIC_SCL_L();
        iic_delay_us(2);
    }
}

uint8_t ssd1306_interface_iic_init(void)
{
    SDA_OUT();
    IIC_SDA_H();
    IIC_SCL_H();
    return 0;
}

uint8_t ssd1306_interface_iic_deinit(void)
{

    return 0;
}

uint8_t ssd1306_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    a_iic_start();
    a_iic_send_byte(addr);
    if (a_iic_wait_ack() != 0)
    {
        a_iic_stop();
        return 1;
    }
    a_iic_send_byte(reg);
    if (a_iic_wait_ack() != 0)
    {
        a_iic_stop();
        return 1;
    }
    for (i = 0; i < len; i++)
    {
        a_iic_send_byte(buf[i]);
        if (a_iic_wait_ack() != 0)
        {
            a_iic_stop();
            return 1;
        }
    }
    a_iic_stop();

    return 0;
}

uint8_t ssd1306_interface_spi_init(void)
{
    return 1;
}

uint8_t ssd1306_interface_spi_deinit(void)
{
    return 1;
}

uint8_t ssd1306_interface_spi_write_cmd(uint8_t *buf, uint16_t len)
{
    return 1;
}

void ssd1306_interface_delay_ms(uint32_t ms)
{
    uint32_t cycles = ms * (CPUCLK_FREQ / 1000) / 4;
    for (volatile uint32_t i = 0; i < cycles; i++) {}
}

void ssd1306_interface_debug_print(const char *const fmt, ...)
{
    (void)fmt;
}

uint8_t ssd1306_interface_spi_cmd_data_gpio_init(void)
{
    return 0;
}

uint8_t ssd1306_interface_spi_cmd_data_gpio_deinit(void)
{
    return 0;
}

uint8_t ssd1306_interface_spi_cmd_data_gpio_write(uint8_t value)
{
    (void)value;
    return 0;
}

uint8_t ssd1306_interface_reset_gpio_init(void)
{
    return 0;
}

uint8_t ssd1306_interface_reset_gpio_deinit(void)
{
    return 0;
}

uint8_t ssd1306_interface_reset_gpio_write(uint8_t value)
{
    if (value != 0)
    {
        DL_GPIO_writePinsVal(IIC_PORT, IIC_RESET_MASK, IIC_RESET_MASK);
    }
    else
    {
        DL_GPIO_writePinsVal(IIC_PORT, IIC_RESET_MASK, 0);
    }
    return 0;
}
