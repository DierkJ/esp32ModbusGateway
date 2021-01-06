/**
**********************************************************************************************************************************************************************************************************************************
* @file:	display.h
*
* @brief:	all OLED display functions
*
* @author:	Dierk Arp
* @date:	20201129 11:36:51 Sunday
* @version:	1.0
*
* @copyright:	(c)2021 Team HAHIS
*
* MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
**********************************************************************************************************************************************************************************************************************************
**/
#ifndef _DISPLAY_H_INCLUDED
#define _DISPLAY_H_INCLUDED

#include <ss_oled.h>

#define DISPLAYREFRESH_MS               40      // OLED refresh cycle in ms [default = 40] -> 1000/40 = 25 frames per second
#define DISPLAYCONTRAST                 80      // 0 .. 255, OLED display contrast [default = 80]
#define DISPLAYCYCLE                    5       // Auto page flip delay in sec [default = 2] for devices without button
#define HOMECYCLE                       30      // house keeping cycle in seconds [default = 30 secs]

// settings for display library
#define USE_BACKBUFFER 1

// setup display hardware type, default is OLED 128x64
#ifndef MY_DISPLAY_TYPE
#define MY_DISPLAY_TYPE OLED_128x64
#endif

#ifndef MY_DISPLAY_SDA
#define MY_DISPLAY_SDA  (21)
#endif

#ifndef MY_DISPLAY_SCL
#define MY_DISPLAY_SCL  (22)
#endif

#ifdef MY_DISPLAY_ADDR
#define OLED_ADDR MY_DISPLAY_ADDR
#else
#define OLED_ADDR -1
#endif

#ifdef MY_DISPLAY_RST
#define OLED_RST MY_DISPLAY_RST
#else
#define OLED_RST -1
#endif


#ifndef MY_DISPLAY_INVERT
#define MY_DISPLAY_INVERT 0
#endif

#ifndef USW_HW_I2C
#define USE_HW_I2C 1
#endif

#ifndef MY_DISPLAY_FLIP
#define MY_DISPLAY_FLIP 0
#endif

#ifndef MY_DISPLAY_WIDTH
#define MY_DISPLAY_WIDTH 128 // Width in pixels of OLED-display, must be 32X
#endif
#ifndef MY_DISPLAY_HEIGHT
#define MY_DISPLAY_HEIGHT 64 // Height in pixels of OLED-display, must be 64X
#endif

// some RGB color definitions
#define Black 0x0000       /*   0,   0,   0 */
#define Navy 0x000F        /*   0,   0, 128 */
#define DarkGreen 0x03E0   /*   0, 128,   0 */
#define DarkCyan 0x03EF    /*   0, 128, 128 */
#define Maroon 0x7800      /* 128,   0,   0 */
#define Purple 0x780F      /* 128,   0, 128 */
#define Olive 0x7BE0       /* 128, 128,   0 */
#define LightGrey 0xC618   /* 192, 192, 192 */
#define DarkGrey 0x7BEF    /* 128, 128, 128 */
#define Blue 0x001F        /*   0,   0, 255 */
#define Green 0x07E0       /*   0, 255,   0 */
#define Cyan 0x07FF        /*   0, 255, 255 */
#define Red 0xF800         /* 255,   0,   0 */
#define Magenta 0xF81F     /* 255,   0, 255 */
#define Yellow 0xFFE0      /* 255, 255,   0 */
#define White 0xFFFF       /* 255, 255, 255 */
#define Orange 0xFD20      /* 255, 165,   0 */
#define GreenYellow 0xAFE5 /* 173, 255,  47 */
#define Pink 0xF81F

#ifndef MY_DISPLAY_FGCOLOR
#define MY_DISPLAY_FGCOLOR White
#endif
#ifndef MY_DISPLAY_BGCOLOR
#define MY_DISPLAY_BGCOLOR Black
#endif

enum dp_page_t 
{
    DP_PAGE_HOME = 0,
    DP_PAGE_METER = 1,
    DP_PAGE_SENSOR = 2, 
    DP_PAGE_TTN = 3,
    DP_PAGE_INTERNAL = 4,
    DP_PAGE_LAST = DP_PAGE_INTERNAL
} ;

extern uint8_t displaybuf[];

void dp_setup(int contrast = 0);
void dp_init(bool verbose = false);
void dp_shutdown(void);
void dp_handle(void);
void dp_drawPage(dp_page_t dp);

void dp_printf(uint16_t x, uint16_t y, uint8_t font, uint8_t inv,
               const char *format, ...);
void dp_dump(uint8_t *pBuffer);
void dp_contrast(uint8_t contrast);
void dp_clear(void);
void dp_power(uint8_t screenon);
void dp_printqr(uint16_t offset_x, uint16_t offset_y, const char *Message);
void dp_fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                 uint8_t bRender);
void dp_scrollHorizontal(uint8_t *buf, const uint16_t width,
                         const uint16_t height, bool left = true);
void dp_scrollVertical(uint8_t *buf, const uint16_t width,
                       const uint16_t height, int offset = 0);
int dp_drawPixel(uint8_t *buf, const uint16_t x, const uint16_t y,
                 const uint8_t dot);
void dp_plotCurve(uint16_t count, bool reset);
void dp_rescaleBuffer(uint8_t *buf, const int factor);
void dp_progressbar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t progress);


#endif