#ifndef ST7735_MIN_H_
#define ST7735_MIN_H_

/*
 * SSD1306 128x64 I2C OLED driver for MSPM0G3507.
 * File/function names are kept as st7735_min.* so the old CCS project
 * can be overwritten without changing project item names.
 *
 * Default pins:
 *   PB9 = LCD_SCL = OLED SCL
 *   PB8 = LCD_SDA = OLED SDA
 *
 * SysConfig requirement:
 *   LCD_SCL and LCD_SDA should be GPIO Output, Initial Value Set,
 *   IO Structure = Standard.
 *   SDA open-drain behavior is emulated in software by switching the
 *   output enable, so PB8/PB9 do not need 5V-tolerant open-drain mode.
 */

#include <stdint.h>

#define LCD_WIDTH   128
#define LCD_HEIGHT  64

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF

void LCD_Init(void);
void LCD_FillScreen(uint16_t color);
void LCD_DrawString(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg);
void LCD_DrawChar(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg);
void LCD_Update(void);
void LCD_TestAllOn(void);
void LCD_TestPattern(void);

#endif /* ST7735_MIN_H_ */

