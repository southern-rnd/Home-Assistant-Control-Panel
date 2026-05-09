#ifndef USER_SETUP_H
#define USER_SETUP_H

#define ILI9341_DRIVER
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_MISO  12
#define TFT_CS    15
#define TFT_DC    33
#define TFT_RST    4
#define TFT_BL    32
#define TFT_BACKLIGHT_ON HIGH
#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY   16000000
#define SPI_TOUCH_FREQUENCY  2500000
#define SMOOTH_FONT
#define FONT_FONT_NOTOSANS

#define LOAD_GLCD   // Font 1 - Adafruit_GFX GLCD font 8x8
#define LOAD_FONT1  // Font 1 - Original 16px additive font
#define LOAD_FONT2  // Font 2 - Small 16px bitmap font
#define LOAD_FONT4  // Font 4 - Medium 26px bitmap font
#define LOAD_FONT6  // Font 4 - Medium 26px bitmap font
#define LOAD_FONT7  // Font 7 - 48px bitmap font
#define LOAD_FONT8  // Font 8 - 75px font
#define LOAD_FONT8B // Font 8 - 75px bold font
#define SMOOTH_FONT

#endif
