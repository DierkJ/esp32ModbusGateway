/**
**********************************************************************************************************************************************************************************************************************************
* @file:	main.cpp
*
* @brief:	Startup for Modbus Gateway
*
* @author:	Dierk Arp
* @date:	20201129 16:01:24 
* @version:	1.0
*
* @copyright:	(c)2020 Team HAHIS
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


#include "main.h"
#include "display.h"
#include "modbus.h"
#include "lorawan.h"
#include "mbhttpserver.h"


#define NTP_SERVER "de.pool.ntp.org"
#define TZ_INFO "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00" // Western European Time

SemaphoreHandle_t I2Caccess;
String g_devicename = "HahisMBGW01";

void setup() 
{
  // create some semaphores for syncing / mutexing tasks
  I2Caccess = xSemaphoreCreateMutex(); // for access management of i2c bus
  assert(I2Caccess != NULL);
  I2C_MUTEX_UNLOCK();


  // Set WiFi to station mode and disconnect from an AP if it was previously connected
/*
  WiFi.mode(WIFI_STA);
  delay(250);
  WiFi.disconnect();
*/

  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_INFO);
 
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  ESP_LOGI(TAG,
             "This is ESP32 chip with %d CPU cores, WiFi%s%s, silicon revision "
             "%d, %dMB %s Flash",
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
             chip_info.revision, spi_flash_get_chip_size() / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                           : "external");
  ESP_LOGI(TAG, "Internal Total heap %d, internal Free Heap %d",
             ESP.getHeapSize(), ESP.getFreeHeap());
  ESP_LOGI(TAG, "SPIRam Total heap %d, SPIRam Free Heap %d",
             ESP.getPsramSize(), ESP.getFreePsram());
  ESP_LOGI(TAG, "ChipRevision %d, Cpu Freq %d, SDK Version %s",
             ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
  ESP_LOGI(TAG, "Flash Size %d, Flash Speed %d", ESP.getFlashChipSize(),
             ESP.getFlashChipSpeed());
  ESP_LOGI(TAG, "Wifi/BT software coexist version %s",
             esp_coex_version_get());

  delay(100);

 
  dp_init(true);
  ESP_LOGI(TAG, "Display init");

  // Open SPI FF
  if(!SPIFFS.begin())
  {
     ESP_LOGI(TAG, "An Error has occurred while mounting SPIFFS");
     return;
  }

  bool res;

#ifdef WIFI_MANAGER
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  // for Test
  // wm.resetSettings();
  res = wm.autoConnect(g_devicename.c_str()); // anonymous ap
  //res = WiFi.isConnected();
#else

    WiFi.mode(WIFI_STA);
    WiFi.begin("$%XY", "lamsenjoch27");
    if (WiFi.waitForConnectResult() != WL_CONNECTED) 
      res = false;
    else
      res = true;

#endif

  if(!res) 
  {
        ESP_LOGI(TAG, "Failed to connect");
        // ESP.restart();
  } 
  else 
  {
        //if you get here you have connected to the WiFi    
        ESP_LOGI(TAG, "connected...yeey :)");
    
        String sIP = WiFi.localIP().toString();
        const char *pS = sIP.c_str();
        ESP_LOGI(TAG, "IP-Adress: %s", pS);
        dp_printf(0, 5, FONT_SMALL, 0, "IP: %s", pS);

        struct tm local;
        configTzTime(TZ_INFO, NTP_SERVER); // ESP32 Systemzeit mit NTP Synchronisieren
        getLocalTime(&local, 10000); 
        
        ESP_LOGI(TAG, "Time: %2.2d:%2.2d:%2.2d", local.tm_hour, local.tm_min, local.tm_sec);
        dp_printf(0, 6, FONT_SMALL, 0, "Time: %2.2d:%2.2d:%2.2d", local.tm_hour, local.tm_min, local.tm_sec);
        dp_dump(displaybuf);

        delay(1000);
        StartModBus();
        StartHTTP();
        otaInit();
        loraInit();
    }
}

// housekeeping
uint32_t _minFreeHeap = 0;
uint32_t _freeHeap = 0;

static long _tsMillis = 0;
static long _loopMillis = 0;

void loop() 
{

  otaHandle();
  ModBusHandle();
  loraHandle();
 
  if ((millis() - _tsMillis) > 1000L)
  {
    // update time
    struct tm local;
    getLocalTime(&local, 10000); 

    dp_printf(0, 6, FONT_SMALL, 0, "Time: %2.2d:%2.2d:%2.2d ", local.tm_hour, local.tm_min, local.tm_sec);
    dp_dump(displaybuf);

  
    if (g_minFreeHeap != _minFreeHeap || ESP.getFreeHeap() != _freeHeap) 
    {
      _freeHeap = ESP.getFreeHeap();
      if (_freeHeap < g_minFreeHeap)
        g_minFreeHeap = _freeHeap;
      _minFreeHeap = g_minFreeHeap;

    }
    _tsMillis = millis();
  }
}