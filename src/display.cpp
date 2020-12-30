/**
**********************************************************************************************************************************************************************************************************************************
* @file:	display.cpp
*
* @brief:	OLED Display functions
*
* @author:	Dierk Arp
* @date:	20201129 11:29:27 Sunday
* @version:	1.0
*
* @copyright:	(c) 2020 Team HAHIS
*
* The reproduction, distribution and utilization of this document
* as well as the communication of its content to others without
* express authorization is prohibited. Offenders will be held liable
* for the payment of damages. All rights reserved in the event of
* the grant of a patent, utility model or design
* Refer to protection notice ISO 16016
*
**********************************************************************************************************************************************************************************************************************************
**/

#include "globals.h"
#include "Bitmaps.h"
#include "display.h"
#include "i2c.h"

static const char TAG[] = __FILE__;

// helper array for converting month values to text
const char *printmonth[] = {"xxx", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
uint8_t DisplayIsOn = 1;    // default: ON
uint8_t displaybuf[MY_DISPLAY_WIDTH * MY_DISPLAY_HEIGHT / 8] = {0};
static uint8_t plotbuf[MY_DISPLAY_WIDTH * MY_DISPLAY_HEIGHT / 8] = {0};

SSOLED ssoled;

/**
 * @brief basic OLED display setup
 * 
 * @param contrast 0..100% (not used)
 */
void dp_setup(int contrast) 
{
  int rc = oledInit(&ssoled, MY_DISPLAY_TYPE, OLED_ADDR, MY_DISPLAY_FLIP,
                    MY_DISPLAY_INVERT, USE_HW_I2C, MY_DISPLAY_SDA,
                    MY_DISPLAY_SCL, OLED_RST,
                    400000L); // use standard I2C bus at 400Khz
  assert(rc != OLED_NOT_FOUND);

  // set display buffer
  oledSetBackBuffer(&ssoled, displaybuf);

  // clear display
  dp_clear();
  if (contrast)
    dp_contrast(contrast);
}

/**
 * @brief Init OLED Display, show startup parameter and Logo
 * 
 * @param verbose true: show startup parameter, false: direct startup with logo
 */
void dp_init(bool verbose) 
{
  // block i2c bus access
  if (!I2C_MUTEX_LOCK())
    ESP_LOGV(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else 
  {

    dp_setup(DISPLAYCONTRAST);

    if (verbose) 
    {

      // show startup screen
      // to come -> display .bmp file with logo

// show chip information
      esp_chip_info_t chip_info;
      esp_chip_info(&chip_info);
      dp_printf(0, 0, 0, 0, "** Hahis ModBus **");
      dp_printf(0, 1, 0, 0, "Software v%s", PROGVERSION);
      dp_printf(0, 3, 0, 0, "ESP32 %d cores", chip_info.cores);
      dp_printf(0, 4, 0, 0, "Chip Rev.%d", chip_info.revision);
      dp_printf(0, 5, 0, 0, "WiFi%s%s",
                (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
 //     dp_printf(0, 6, 0, 0, "%dMB %s Flash",
 //               spi_flash_get_chip_size() / (1024 * 1024),
 //               (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "int."
 //                                                             : "ext.");

      // give user some time to read or take picture
      dp_dump(displaybuf);
      delay(2000);
      dp_clear();
    } // verbose

    dp_clear();
    memset(displaybuf, 0, sizeof(displaybuf));
    memcpy(displaybuf, (const void *)MESSRING_LOGO, sizeof(MESSRING_LOGO) );
    dp_printf(0, 5, FONT_SMALL, 0, "...connecting..." );
    dp_printf(0, 6, FONT_SMALL, 0, "AP: 192.168.4.1");
    dp_dump(displaybuf);

    delay (2000);
    dp_power(DisplayIsOn); // set display off if disabled

    I2C_MUTEX_UNLOCK(); // release i2c bus access mutex
  }                     
} // dp_init

void dp_refresh(bool nextPage) 
{

#ifndef HAS_BUTTON
  static uint32_t framecounter = 0;
#endif

// if display is switched off we don't refresh it to relax cpu
//  if (!DisplayIsOn && (DisplayIsOn == cfg.screenon))
//    return;

 const time_t t = 0;
      
  // block i2c bus access
  if (!I2C_MUTEX_LOCK())
    ESP_LOGV(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else 
  {

#ifndef HAS_BUTTON
    // auto flip page if we are in unattended mode
    if ((++framecounter) > (DISPLAYCYCLE * 1000 / DISPLAYREFRESH_MS)) {
      framecounter = 0;
      nextPage = true;
    }
#endif

    dp_drawPage(t, nextPage);
    dp_dump(displaybuf);

    I2C_MUTEX_UNLOCK(); // release i2c bus access

  } // mutex
} // refreshDisplay()

void dp_drawPage(time_t t, bool nextpage) 
{

  // write display content to display buffer
  // nextpage = true -> flip 1 page

  static uint8_t DisplayPage = 0;

  // line 1/2: pax counter
  dp_printf(0, 0, FONT_STRETCHED, 0, "ModBus");
 
start:

  if (nextpage) 
  {
    DisplayPage = (DisplayPage >= DISPLAY_PAGES - 1) ? 0 : (DisplayPage + 1);
    dp_clear();
  }

  switch (DisplayPage) 
  {

    // page 0: parameters overview
    // ...
    // page N: blank screen

    // page 0: parameters overview
  case 0:
    DisplayPage++; // next page
    break;

  default:
    goto start; // start over

  } // switch

} // dp_drawPage

// display helper functions
void dp_printf(uint16_t x, uint16_t y, uint8_t font, uint8_t inv,
               const char *format, ...) 
{
  char loc_buf[64];
  char *temp = loc_buf;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);
  int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
  va_end(copy);
  if (len < 0) {
    va_end(arg);
    return;
  };
  if (len >= sizeof(loc_buf)) {
    temp = (char *)malloc(len + 1);
    if (temp == NULL) {
      va_end(arg);
      return;
    }
    len = vsnprintf(temp, len + 1, format, arg);
  }
  va_end(arg);
  oledWriteString(&ssoled, 0, x, y, temp, font, inv, false);
  if (temp != loc_buf) {
    free(temp);
  }
}

void dp_dump(uint8_t *pBuffer) 
{
  oledDumpBuffer(&ssoled, pBuffer);
}

void dp_clear() 
{
  oledFill(&ssoled, 0, 1);
}

void dp_contrast(uint8_t contrast) 
{
  oledSetContrast(&ssoled, contrast);
}

void dp_power(uint8_t screenon) 
{
  oledPower(&ssoled, screenon);
}

void dp_shutdown(void) 
{
  // block i2c bus access
  if (!I2C_MUTEX_LOCK())
    ESP_LOGV(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else {
    oledPower(&ssoled, false);
    delay(DISPLAYREFRESH_MS / 1000 * 1.1);
    I2C_MUTEX_UNLOCK(); // release i2c bus access
  }
}


void dp_fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                 uint8_t bRender) {
  for (uint16_t xi = x; xi < x + width; xi++)
    oledDrawLine(&ssoled, xi, y, xi, y + height - 1, bRender);
}

int dp_drawPixel(uint8_t *buf, const uint16_t x, const uint16_t y,
                 const uint8_t dot) {

  if (x > MY_DISPLAY_WIDTH || y > MY_DISPLAY_HEIGHT)
    return -1;

  uint8_t bit = y & 7;
  uint16_t idx = y / 8 * MY_DISPLAY_WIDTH + x;

  buf[idx] &= ~(1 << bit); // clear pixel
  if (dot)
    buf[idx] |= (1 << bit); // set pixel

  return 0;
}

void dp_scrollHorizontal(uint8_t *buf, const uint16_t width,
                         const uint16_t height, bool left) {

  uint16_t col, page, idx = 0;

  for (page = 0; page < height / 8; page++) {
    if (left) { // scroll left
      for (col = 0; col < width - 1; col++) {
        idx = page * width + col;
        buf[idx] = buf[idx + 1];
      }
      buf[idx + 1] = 0;
    } else // scroll right
    {
      for (col = width - 1; col > 0; col--) {
        idx = page * width + col;
        buf[idx] = buf[idx - 1];
      }
      buf[idx - 1] = 0;
    }
  }
}

void dp_scrollVertical(uint8_t *buf, const uint16_t width,
                       const uint16_t height, int offset) {

  uint64_t buf_col;

  if (!offset)
    return; // nothing to do

  for (uint16_t col = 0; col < MY_DISPLAY_WIDTH; col++) {
    // convert column bytes from display buffer to uint64_t
    buf_col = *(uint64_t *)&buf[col * MY_DISPLAY_HEIGHT / 8];

    if (offset > 0) // scroll down
      buf_col <<= offset;
    else // scroll up
      buf_col >>= abs(offset);

    // write back uint64_t to uint8_t display buffer
    *(uint64_t *)&buf[col * MY_DISPLAY_HEIGHT / 8] = buf_col;
  }
}

void dp_plotCurve(uint16_t count, bool reset) {

  static uint16_t last_count = 0, col = 0, row = 0;
  uint16_t v_scroll = 0;

  if ((last_count == count) && !reset)
    return;

  if (reset) {                      // next count cycle?
    if (col < MY_DISPLAY_WIDTH - 1) // matrix not full -> increment column
      col++;
    else // matrix full -> scroll left 1 dot
      dp_scrollHorizontal(plotbuf, MY_DISPLAY_WIDTH, MY_DISPLAY_HEIGHT, true);

  } else // clear current dot
    dp_drawPixel(plotbuf, col, row, 0);

  // scroll down, if necessary
  while ((count - v_scroll) > MY_DISPLAY_HEIGHT - 1)
    v_scroll++;
  if (v_scroll)
    dp_scrollVertical(plotbuf, MY_DISPLAY_WIDTH, MY_DISPLAY_HEIGHT, v_scroll);

  // set new dot
  // row = MY_DISPLAY_HEIGHT - 1 - (count - v_scroll) % MY_DISPLAY_HEIGHT;
  row = MY_DISPLAY_HEIGHT - 1 - count - v_scroll;
  last_count = count;
  dp_drawPixel(plotbuf, col, row, 1);
}

/**
 * @brief displays a progressbar
 * 
 * @param x       origin x
 * @param y       origin y
 * @param width   
 * @param height 
 * @param progress  0..100,  no limit checks 
 */
void dp_progressbar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t progress)
{
    // draw box
    oledDrawLine(&ssoled, x, y, x+width, y, false);
    oledDrawLine(&ssoled, x, y+height, x+width, y+height, false);
    oledDrawLine(&ssoled, x, y, x, y+height, false);
    oledDrawLine(&ssoled, x+width, y, x+width, y+height, false);

    // fill progress
    uint16_t maxProgressWidth = (width-2) * progress / 100;
    dp_fillRect (x+1, y+1, maxProgressWidth, height-2, false);
}
