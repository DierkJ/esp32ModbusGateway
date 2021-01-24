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
* @copyright:	(c) 2021 Team HAHIS
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

/*
 * All display printing / Grafics / etc. is done here in this module
 * The different worker modules are exporting the relevant data to be displayed.
 * 
 * the Display has following pages:
 * 1: (Home) Logo / IP Adress
 * 2: Meter Data
 * 3: Sensor Data
 * 4: TTN Data
 * 5: internal data
 */  


static dp_page_t curDisplayPage;

/**
 * @brief basic OLED display setup
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
    ESP_LOGI(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
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
      dp_printf(0, 0, 0, 0, "** ModBus Gateway **");
      dp_printf(0, 1, 0, 0, "Software v%s", VERSION);
      dp_printf(0, 3, 0, 0, "ESP32 %d cores", chip_info.cores);
      dp_printf(0, 4, 0, 0, "Chip Rev.%d", chip_info.revision);
      dp_printf(0, 5, 0, 0, "WiFi%s%s",
                (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
 
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

    delay (1000);
    dp_power(DisplayIsOn); // set display off if disabled

    I2C_MUTEX_UNLOCK(); // release i2c bus access mutex
  }     
  curDisplayPage = DP_PAGE_HOME;                
} 

static long _dpMillis = 0L;

void dp_handle(void)
{
  if ((millis() - _dpMillis) > DISPLAYCYCLE * 1000L)
  {
    curDisplayPage = (dp_page_t) (curDisplayPage + 1);
    if (curDisplayPage > DP_PAGE_LAST)
      curDisplayPage = DP_PAGE_HOME;
    dp_drawPage (curDisplayPage);
    _dpMillis = millis();
  }
}

void dp_drawPage(dp_page_t dp) 
{
  if (!I2C_MUTEX_LOCK())
    ESP_LOGI(TAG,  "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else 
  {
    dp_clear();
    memset(displaybuf, 0, sizeof(displaybuf));
    
    switch(dp)
    {
      case DP_PAGE_HOME:
        dp_printf(0, 0, FONT_NORMAL, 0, "ModBus Gateway" );
        dp_printf(0, 3, FONT_SMALL, 0, "IP:   %s", g_ipAddress.c_str() );
        dp_printf(0, 4, FONT_SMALL, 0, "Mask: %s", g_ipSubNet.c_str() );
        dp_printf(0, 5, FONT_SMALL, 0, "SSID: %s", g_SSID );
        dp_printf(0, 6, FONT_SMALL, 0, "DNS:  %s", g_devicename.c_str() );

        break;

      case DP_PAGE_METER:
        dp_printf(0, 0, FONT_NORMAL, 0, "Power Meter" );
        if (g_modBusMeterData.fConnected)
        {
          dp_printf(0, 3, FONT_SMALL, 0, "Power:   %.0f W", g_modBusMeterData.fPhasePower[0]);
          dp_printf(0, 4, FONT_SMALL, 0, "In:      %.1f kWh", g_modBusMeterData.fEnergyIn );
          dp_printf(0, 5, FONT_SMALL, 0, "Out:     %.1f kWh", g_modBusMeterData.fEnergyOut );
          dp_printf(0, 6, FONT_SMALL, 0, "Line:    %.1f V", g_modBusMeterData.fPhaseVoltage[0] );
        }
        else
          dp_printf(0, 4, FONT_SMALL, 0, "not connected" );
        break;

      case DP_PAGE_SENSOR:
        dp_printf(0, 0, FONT_NORMAL, 0, "Environment" );
        dp_printf(0, 3, FONT_SMALL, 0, "Temp:     %.1f C", g_SensorData.temperature);
        dp_printf(0, 4, FONT_SMALL, 0, "Pressure: %.1f hPa", g_SensorData.pressure );
        dp_printf(0, 5, FONT_SMALL, 0, "Alti:     %.1f m", g_SensorData.altitude );
        break;

      case DP_PAGE_TTN:
        dp_printf(0, 0, FONT_NORMAL, 0, "TTN" );
        dp_printf(0, 3, FONT_SMALL, 0, "NetID:    %d", g_LoraData.netid );
        dp_printf(0, 4, FONT_SMALL, 0, "DevAddr:  0x%8.8x", g_LoraData.devaddr );
        dp_printf(0, 5, FONT_SMALL, 0, "Tx  :     %d (%d)", g_LoraData.nTX, g_LoraData.nAck );
        dp_printf(0, 6, FONT_SMALL, 0, "Rx  :     %d", g_LoraData.nRX );
        break;

      case DP_PAGE_INTERNAL:
        dp_printf(0, 0, FONT_NORMAL, 0, "Internal" );
        dp_printf(0, 3, FONT_SMALL, 0, "Up: %s", getUptimeString().c_str());
        dp_printf(0, 4, FONT_SMALL, 0, "free Heap: %d", g_minFreeHeap);
        dp_printf(0, 5, FONT_SMALL, 0, "Version: V%s", VERSION);
        //dp_printf(0, 6, FONT_SMALL, 0, "Last Acc:  %d", g_lastAccessTime);
        break;
    }
    // 
    struct tm local;
    getLocalTime(&local, 10000); 
    dp_printf(128-(7*8), 7, FONT_SMALL, 0, "%2.2d:%2.2d:%2.2d ", local.tm_hour, local.tm_min, local.tm_sec);
    dp_dump(displaybuf);
  }

  I2C_MUTEX_UNLOCK(); // release i2c bus access mutex
} 

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
    ESP_LOGI(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
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

