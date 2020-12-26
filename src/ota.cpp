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
static const char TAG[] = __FILE__;

#include <ESPmDNS.h>
#include <ArduinoOTA.h>

#include "globals.h"
#include "ota.h"

void otaDisplayStart()
{
    dp_clear();
    dp_printf(0, 0, FONT_LARGE, 0, "OTA" );
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
    if ((uProg100 % 10) == 0)
      dp_progressbar(5, MY_DISPLAY_HEIGHT-20, MY_DISPLAY_WIDTH-10, 18, uProg100);
    
    dp_dump(displaybuf);
}

void otaDisplayError(ota_error_t error, char *szText) 
{
    dp_clear();
    dp_printf(0, 0, FONT_LARGE, 0, "OTA" );
    dp_printf(0, 5, FONT_SMALL, 0, "Error: %s (%d)", szText, error);
    dp_dump(displaybuf);
}

void otaInit() 
{

#ifdef OTA_DIRECT 
  ArduinoOTA.setHostname(g_devicename.c_str());

  ArduinoOTA.onStart([]() 
  {
    ESP_LOGI(TAG, "OTA_DIRECT: onStart");
    otaDisplayStart();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    ESP_LOGI(TAG, "OTA_DIRECT: onProgress: %d of %d", progress, total);
    otaDisplayProgress(progress, total);
  });
  ArduinoOTA.onEnd([]() 
  {
    ESP_LOGI(TAG, "OTA_DIRECT: onEnd");
    otaDisplayEnd();
  });

  ArduinoOTA.onError([](ota_error_t error) 
  {
    ESP_LOGI(TAG, "OTA_DIRECT: onError: %d", error);
    
    switch (error)
    {
      case OTA_AUTH_ERROR:  
        otaDisplayError( error, "Auth Failed");
        break;
      case OTA_BEGIN_ERROR:  
        otaDisplayError( error, "Begin Failed");
        break;
      case OTA_CONNECT_ERROR:  
        otaDisplayError( error, "Connect Failed");
        break;
      case OTA_RECEIVE_ERROR:  
        otaDisplayError( error, "RX Failed");
        break;
      case OTA_END_ERROR:  
        otaDisplayError( error, "End Failed");
        break;
      default:  
        otaDisplayError( error, "unknown");
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

