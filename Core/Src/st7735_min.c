/*
 * st7735_min.c - SSD1306 128x64 I2C OLED driver for MSPM0G3507
 *
 * The file name is kept as st7735_min.c so the existing CCS project can
 * be overwritten without changing project item names.
 *
 * Pins used as software I2C:
 *   PB9  LCD_SCL -> OLED SCL
 *   PB8  LCD_SDA -> OLED SDA
 *
 * SysConfig requirement:
 *   LCD_SCL: GPIO Output, Initial Value Set, IO Structure Standard.
 *   LCD_SDA: GPIO Output, Initial Value Set, IO Structure Standard.
 *
 * The code emulates I2C open-drain behavior on SDA by disabling the
 * output driver when the bus needs a logic high. This avoids SysConfig
 * errors on pins that do not support 5V-tolerant open-drain mode.
 */
#include "ti_msp_dl_config.h"
#include "st7735_min.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef CPU_HZ
#define CPU_HZ (32000000UL)
#endif

#define OLED_PORT       (GPIOB)
#define OLED_SCL_PIN    (DL_GPIO_PIN_9)
#define OLED_SDA_PIN    (DL_GPIO_PIN_8)

/* Most SSD1306 I2C OLED modules use 0x3C. Some use 0x3D.
 * This driver auto-tests 0x3C first, then 0x3D.
 */
#define OLED_ADDR_0     (0x3C)
#define OLED_ADDR_1     (0x3D)

#define OLED_PAGES      (LCD_HEIGHT / 8U)
#define OLED_BUF_SIZE   (LCD_WIDTH * OLED_PAGES)

/*
 * SCL is driven as normal push-pull GPIO. This is acceptable for a single
 * master driving a simple OLED module without clock stretching.
 *
 * SDA is emulated open-drain:
 *   - low:  drive output low
 *   - high: release the output driver and let the OLED module pull-up hold high
 */
#define SCL_HIGH()      DL_GPIO_setPins(OLED_PORT, OLED_SCL_PIN)
#define SCL_LOW()       DL_GPIO_clearPins(OLED_PORT, OLED_SCL_PIN)
#define SDA_RELEASE()   do { \
                            DL_GPIO_setPins(OLED_PORT, OLED_SDA_PIN); \
                            DL_GPIO_disableOutput(OLED_PORT, OLED_SDA_PIN); \
                        } while (0)
#define SDA_LOW()       do { \
                            DL_GPIO_clearPins(OLED_PORT, OLED_SDA_PIN); \
                            DL_GPIO_enableOutput(OLED_PORT, OLED_SDA_PIN); \
                        } while (0)
#define SDA_READ()      ((DL_GPIO_readPins(OLED_PORT, OLED_SDA_PIN) & OLED_SDA_PIN) ? 1U : 0U)

static uint8_t g_oled_buf[OLED_BUF_SIZE];
static uint8_t g_oled_addr = OLED_ADDR_0;

static void oled_delay_ms(uint32_t ms)
{
    while (ms--) {
        DL_Common_delayCycles(CPU_HZ / 1000U);
    }
}

static void i2c_delay(void)
{
    /* About a few microseconds at 32 MHz. Slow enough for first bring-up. */
    DL_Common_delayCycles(120);
}

static void i2c_start(void)
{
    SDA_RELEASE();
    SCL_HIGH();
    i2c_delay();
    SDA_LOW();
    i2c_delay();
    SCL_LOW();
    i2c_delay();
}

static void i2c_stop(void)
{
    SDA_LOW();
    i2c_delay();
    SCL_HIGH();
    i2c_delay();
    SDA_RELEASE();
    i2c_delay();
}

static bool i2c_write_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x80U) {
            SDA_RELEASE();
        } else {
            SDA_LOW();
        }
        i2c_delay();
        SCL_HIGH();
        i2c_delay();
        SCL_LOW();
        i2c_delay();
        data <<= 1;
    }

    /* ACK bit: release SDA, slave pulls it low. */
    SDA_RELEASE();
    i2c_delay();
    SCL_HIGH();
    i2c_delay();
    bool ack = (SDA_READ() == 0U);
    SCL_LOW();
    i2c_delay();
    return ack;
}

static bool oled_write_cmd_to(uint8_t addr, uint8_t cmd)
{
    bool ok;
    i2c_start();
    ok  = i2c_write_byte((uint8_t)(addr << 1));  /* write */
    ok &= i2c_write_byte(0x00);                  /* control byte: command */
    ok &= i2c_write_byte(cmd);
    i2c_stop();
    return ok;
}

static bool oled_cmd(uint8_t cmd)
{
    return oled_write_cmd_to(g_oled_addr, cmd);
}

static bool oled_write_data_page(uint8_t page, const uint8_t *data)
{
    bool ok;

    oled_cmd((uint8_t)(0xB0U + page));  /* Page address */
    oled_cmd(0x00);                     /* Lower column address */
    oled_cmd(0x10);                     /* Higher column address */

    i2c_start();
    ok  = i2c_write_byte((uint8_t)(g_oled_addr << 1));
    ok &= i2c_write_byte(0x40);         /* control byte: data */
    for (uint16_t i = 0; i < LCD_WIDTH; i++) {
        ok &= i2c_write_byte(data[i]);
    }
    i2c_stop();
    return ok;
}

static bool oled_detect_addr(uint8_t addr)
{
    i2c_start();
    bool ack = i2c_write_byte((uint8_t)(addr << 1));
    i2c_stop();
    return ack;
}

static void oled_select_address(void)
{
    /* If ACK read works, auto-select address. If it does not, keep 0x3C. */
    if (oled_detect_addr(OLED_ADDR_0)) {
        g_oled_addr = OLED_ADDR_0;
    } else if (oled_detect_addr(OLED_ADDR_1)) {
        g_oled_addr = OLED_ADDR_1;
    } else {
        g_oled_addr = OLED_ADDR_0;
    }
}

void LCD_Update(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        oled_write_data_page(page, &g_oled_buf[page * LCD_WIDTH]);
    }
}

static void oled_set_pixel(uint16_t x, uint16_t y, bool on)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }

    uint16_t index = x + (y / 8U) * LCD_WIDTH;
    uint8_t bit = (uint8_t)(1U << (y & 0x07U));

    if (on) {
        g_oled_buf[index] |= bit;
    } else {
        g_oled_buf[index] &= (uint8_t)~bit;
    }
}

void LCD_FillScreen(uint16_t color)
{
    memset(g_oled_buf, (color == COLOR_BLACK) ? 0x00 : 0xFF, sizeof(g_oled_buf));
    LCD_Update();
}

/* 5x7 ASCII font, column format, bit0 = top. Covers common BMS display chars. */
static const uint8_t *font5x7(char c)
{
    static const uint8_t blank[5] = {0x00,0x00,0x00,0x00,0x00};
    static const uint8_t colon[5] = {0x00,0x36,0x36,0x00,0x00};
    static const uint8_t dot[5]   = {0x00,0x60,0x60,0x00,0x00};
    static const uint8_t pct[5]   = {0x62,0x64,0x08,0x13,0x23};
    static const uint8_t slash[5] = {0x20,0x10,0x08,0x04,0x02};
    static const uint8_t minus[5] = {0x08,0x08,0x08,0x08,0x08};
    static const uint8_t ques[5]  = {0x02,0x01,0x51,0x09,0x06};

    static const uint8_t num[10][5] = {
        {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
        {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
        {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
        {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
        {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}
    };

    static const uint8_t A[5] = {0x7E,0x11,0x11,0x11,0x7E};
    static const uint8_t B[5] = {0x7F,0x49,0x49,0x49,0x36};
    static const uint8_t C[5] = {0x3E,0x41,0x41,0x41,0x22};
    static const uint8_t D[5] = {0x7F,0x41,0x41,0x22,0x1C};
    static const uint8_t E[5] = {0x7F,0x49,0x49,0x49,0x41};
    static const uint8_t F[5] = {0x7F,0x09,0x09,0x09,0x01};
    static const uint8_t G[5] = {0x3E,0x41,0x49,0x49,0x7A};
    static const uint8_t H[5] = {0x7F,0x08,0x08,0x08,0x7F};
    static const uint8_t I[5] = {0x00,0x41,0x7F,0x41,0x00};
    static const uint8_t J[5] = {0b1110,0b100,0b100,0b100100,0b11000};//handimake
    static const uint8_t K[5] = {0b100100,0b101000,0b110000,0b101000,0b101000};//handimake
    static const uint8_t L[5] = {0x7F,0x40,0x40,0x40,0x40};
    static const uint8_t M[5] = {0x7F,0x02,0x0C,0x02,0x7F};
    static const uint8_t N[5] = {0x7F,0x04,0x08,0x10,0x7F};
    static const uint8_t O[5] = {0x3E,0x41,0x41,0x41,0x3E};
    static const uint8_t P[5] = {0x7F,0x09,0x09,0x09,0x06};
    static const uint8_t Q[5] = {0x3E,0x41,0b01001001,0x3E,0b00001000};//handimake
    static const uint8_t R[5] = {0x7F,0x09,0x19,0x29,0x46};
    static const uint8_t S[5] = {0x46,0x49,0x49,0x49,0x31};
    static const uint8_t T[5] = {0x01,0x01,0x7F,0x01,0x01};
    static const uint8_t U[5] = {0x3F,0x40,0x40,0x40,0x3F};
    static const uint8_t V[5] = {0x1F,0x20,0x40,0x20,0x1F};
    static const uint8_t W[5] = {0b1001001,0b1001001,0b1001001,0b110110,0b101000};//handimake
    static const uint8_t X[5] = {0b10010,0b10100,0b1000,0b10100,0b10010};//handimake
    static const uint8_t Y[5] = {0x07,0x08,0x70,0x08,0x07};
    static const uint8_t Z[5] = {0b11110,0b00010,0b00100,0b01000,0b11110};//handimake

    if (c >= '0' && c <= '9') return num[c - '0'];
    if (c >= 'a' && c <= 'z') c = (char)(c - 32);

    switch (c) {
        case ' ': return blank; case ':': return colon; case '.': return dot;
        case '%': return pct;   case '/': return slash; case '-': return minus; case '?': return ques;
        case 'A': return A; case 'B': return B; case 'C': return C; case 'D': return D;
        case 'E': return E; case 'F': return F; case 'G': return G; case 'H': return H;
        case 'I': return I; case 'J': return J; case 'K': return K; case 'L': return L; 
        case 'M': return M; case 'N': return N; case 'O': return O; case 'P': return P;
        case 'Q': return Q; case 'R': return R; case 'S': return S; case 'T': return T; 
        case 'U': return U; case 'V': return V; case 'W': return W; case 'X': return X; 
        case 'Y': return Y; case 'Z': return Z;
        default:  return ques;
    }
}

void LCD_DrawChar(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg)
{
    const uint8_t *glyph = font5x7(c);
    bool fg_on = (fg != COLOR_BLACK);
    bool bg_on = (bg != COLOR_BLACK);

    for (uint8_t col = 0; col < 6; col++) {
        uint8_t bits = (col < 5U) ? glyph[col] : 0x00;
        for (uint8_t row = 0; row < 8; row++) {
            bool pixel_on = (bits & (1U << row)) ? fg_on : bg_on;
            oled_set_pixel(x + col, y + row, pixel_on);
        }
    }
}

void LCD_DrawString(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg)
{
    while (*s && x < LCD_WIDTH) {
        LCD_DrawChar(x, y, *s++, fg, bg);
        x += 6U;
    }
    LCD_Update();
}

void LCD_TestAllOn(void)
{
    oled_cmd(0xA5);  /* Entire display ON, ignore RAM. */
}

void LCD_TestPattern(void)
{
    memset(g_oled_buf, 0x00, sizeof(g_oled_buf));
    for (uint16_t x = 0; x < LCD_WIDTH; x++) {
        for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
            if (((x / 8U) + (y / 8U)) & 1U) {
                oled_set_pixel(x, y, true);
            }
        }
    }
    LCD_Update();
}

void LCD_Init(void)
{
    SDA_RELEASE();
    SCL_HIGH();
    oled_delay_ms(50);

    oled_select_address();

    oled_cmd(0xAE);       /* Display OFF */
    oled_cmd(0xD5); oled_cmd(0x80);  /* Display clock divide */
    oled_cmd(0xA8); oled_cmd(0x3F);  /* Multiplex ratio: 1/64 */
    oled_cmd(0xD3); oled_cmd(0x00);  /* Display offset */
    oled_cmd(0x40);       /* Start line 0 */
    oled_cmd(0x8D); oled_cmd(0x14);  /* Charge pump ON */
    oled_cmd(0x20); oled_cmd(0x02);  /* Page addressing mode */
    oled_cmd(0xA1);       /* Segment remap */
    oled_cmd(0xC8);       /* COM scan direction remapped */
    oled_cmd(0xDA); oled_cmd(0x12);  /* COM pins for 128x64 */
    oled_cmd(0x81); oled_cmd(0xCF);  /* Contrast */
    oled_cmd(0xD9); oled_cmd(0xF1);  /* Pre-charge */
    oled_cmd(0xDB); oled_cmd(0x40);  /* VCOMH deselect */
    oled_cmd(0xA4);       /* Resume RAM content display */
    oled_cmd(0xA6);       /* Normal display */
    oled_cmd(0x2E);       /* Deactivate scroll */
    oled_cmd(0xAF);       /* Display ON */

    LCD_FillScreen(COLOR_BLACK);
}

