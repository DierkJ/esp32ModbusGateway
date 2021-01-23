/**
**********************************************************************************************************************************************************************************************************************************
* @file:	ota.cpp
*
* @brief:	ota functions for Modbus Gateway
*
*           different methodes for OTA:
*
*           1:  direct OTA per TCP / ESP32 
*           2:  Webupdate from file
*           3:  automatic per Backend
*
*
* @author:	Dierk Arp
* @date:	20201224 16:01:24 
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
static const char TAG[] = __FILE__;

#include <ESPmDNS.h>
#include <ArduinoOTA.h>

#include "globals.h"
#include "ota.h"

#include <ArduinoOTA.h>

/*
**  local status functions (oled / web)
*/
void otaDisplayStart()
{
    dp_clear();
    dp_printf(0, 0, FONT_LARGE, 0, "*Update*" );
    dp_printf(0, 6, FONT_SMALL, 0, "Starting...");
    dp_dump(displaybuf);
}

void otaDisplayEnd()
{
    dp_clear();
    dp_printf(0, 0, FONT_LARGE, 0, ".Reboot.");
    dp_dump(displaybuf);
}

void otaDisplayProgress(unsigned int progress, unsigned int total)
{
    uint16_t uProg100 = progress * 100 / total;

    if (uProg100 < 2)
      dp_printf(0, 6, FONT_SMALL, 0, "          ");
    if ((uProg100 % 5) == 0)
    {
      dp_printf(MY_DISPLAY_WIDTH/2 - 10, 4, FONT_SMALL, 0, "    ");
      dp_printf(MY_DISPLAY_WIDTH/2 - 10, 4, FONT_SMALL, 0, "%d %%", uProg100 );
      dp_progressbar(5, MY_DISPLAY_HEIGHT-20, MY_DISPLAY_WIDTH-10, 18, uProg100);
    }
    dp_dump(displaybuf);
}

void otaDisplayError(int error, char *szText) 
{
    dp_clear();
    dp_printf(0, 0, FONT_LARGE, 0, "OTA" );
    if (szText == NULL)
      dp_printf(0, 5, FONT_SMALL, 0, "Error: %d", error);
    else
      dp_printf(0, 5, FONT_SMALL, 0, "Error: %s (%d)", szText, error);
    
    dp_dump(displaybuf);
}

/**
 * @brief call OTA initialization on startup
 * 
 */
void otaInit() 
{
#ifdef OTA_DIRECT 
  ArduinoOTA.setHostname(g_devicename.c_str());

  ArduinoOTA.onStart([]() 
  {
    debugD( "OTA_DIRECT: onStart");
    otaDisplayStart();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    debugD( "OTA_DIRECT: onProgress: %d of %d", progress, total);
    otaDisplayProgress(progress, total);
  });
  ArduinoOTA.onEnd([]() 
  {
    debugD( "OTA_DIRECT: onEnd");
    otaDisplayEnd();
  });

  ArduinoOTA.onError([](ota_error_t error) 
  {
    debugD( "OTA_DIRECT: onError: %d", error);

    switch (error)
    {
      case OTA_AUTH_ERROR:  
        otaDisplayError( (int)error, "Auth Failed");
        break;
      case OTA_BEGIN_ERROR:  
        otaDisplayError( (int)error, "Begin Failed");
        break;
      case OTA_CONNECT_ERROR:  
        otaDisplayError( (int)error, "Connect Failed");
        break;
      case OTA_RECEIVE_ERROR:  
        otaDisplayError( (int)error, "RX Failed");
        break;
      case OTA_END_ERROR:  
        otaDisplayError( (int)error, "End Failed");
        break;
      default:  
        otaDisplayError( (int)error, "unknown");
        break;
    }
  });
  ArduinoOTA.begin();

#endif


}

void otaHandle()
{

#ifdef OTA_SERVER
    ArduinoOTA.handle();
#endif


}

