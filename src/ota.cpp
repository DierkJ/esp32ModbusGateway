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

#include <ArduinoOTA.h>

#ifdef OTA_IAS

#define MODEBUTTON 0
#include <IOTAppStory.h>		            // IOTAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);	// Initialize IotAppStory

#endif


/*
**  local status functions (oled / web)
*/
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

#ifdef OTA_IAS
  IAS.preSetDeviceName(g_devicename);                       // preset deviceName this is also your MDNS responder: http://'g_devicename'.local
  //IAS.preSetAutoUpdate(false);                            // automaticUpdate (true, false)
  //IAS.preSetAutoConfig(false);                            // automaticConfig (true, false)
  
  IAS.onModeButtonShortPress([]() {
    ESP_LOGI(TAG," If mode button is released, I will enter in firmware update mode.");
  });

  IAS.onModeButtonLongPress([]() {
    ESP_LOGI(TAG," If mode button is released, I will enter in configuration mode.");
  });

  IAS.onModeButtonVeryLongPress([]() {
    ESP_LOGI(TAG," If mode button is released, I will enter in ??? mode.");
  });

  IAS.onFirmwareUpdateProgress([](int written, int total)
  {
    otaDisplayProgress(written, total);
  });
  
  IAS.onModeButtonNoPress([]() {
    ESP_LOGI(TAG," Mode button is not pressed");
  });
  
  IAS.onFirstBoot([]() {                              
    ESP_LOGI(TAG,"onFirstBoot");
  });

  IAS.onFirmwareUpdateCheck([]() {
    ESP_LOGI(TAG,"Checking if there is a firmware update available.");
  });

  IAS.onFirmwareUpdateDownload([]() {
    ESP_LOGI(TAG,"Downloading and Installing firmware update.");
    otaDisplayStart();
  });

  IAS.onFirmwareUpdateError([]() {
    ESP_LOGI(TAG,"pdate failed...Check your logs");
    otaDisplayError(55, NULL);
  });

  IAS.onConfigMode([]() {
    ESP_LOGI(TAG,"Starting configuration mode. Search for my WiFi and connect to 192.168.4.1.");
  });
   
  IAS.begin();                                   // Run IOTAppStory
  IAS.setCallHomeInterval(120);                  // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production

#endif

}

void otaHandle()
{

#ifdef OTA_SERVER
    ArduinoOTA.handle();
#endif

#ifdef OTA_IAS
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)
#endif


}

