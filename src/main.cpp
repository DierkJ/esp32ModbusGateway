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
static const char TAG[] = __FILE__;


#include "main.h"
#include "version.h"
#include "display.h"
#include "modbus.h"
#include "lorawan.h"
#include "mbhttpserver.h"
#include "i2c.h"
#include "util.h"
#include "PersistentConfig.h"


#define NTP_SERVER "de.pool.ntp.org"
#define TZ_INFO "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00" // Western European Time

SemaphoreHandle_t I2Caccess;
String g_devicename = "HahisMBGW01";
String g_ipAddress ;
String g_ipSubNet;
String g_SSID;
struct tm g_bootTime;

PersistentConfig g_cfg;

/*
** remote debugger 
** Telnet / Web / Serial
*/
RemoteDebug Debug;


/*
** some helper functions
*/


void setup() 
{
  // create some semaphores for syncing / mutexing tasks
  I2Caccess = xSemaphoreCreateMutex(); // for access management of i2c bus
  assert(I2Caccess != NULL);
  I2C_MUTEX_UNLOCK();


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

  ESP_LOGI(TAG, "Cause %d:   %s\n", getResetReason(0), getResetReasonStr(0));
  ESP_LOGI(TAG, "Chip ID:    %05X\n", getChipId());
  ESP_LOGI(TAG, "Version v%s, built %s\n", VERSION, BUILD_TIMESTAMP);
  delay(100);

  // open i2c bus
  i2c_init();
 
  dp_init(true);
  ESP_LOGI(TAG, "Display init");

  // scan i2c bus for devices
  i2c_scan();

  // Open SPI FF
  if(SPIFFS.begin())
  {
    // Checks moved to http module
  }
  else
  {
     ESP_LOGI(TAG, "An Error has occurred while mounting SPIFFS");
     return;
  }

  bool res;
  res = g_cfg.Load();
  if (!res)
  {
    ESP_LOGE(TAG, "no Configuration :-(");
  }

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
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);      
        
        g_ipAddress = WiFi.localIP().toString();
        g_ipSubNet = WiFi.subnetMask().toString();
        g_SSID = WiFi.SSID();

        struct tm local;
        configTzTime(TZ_INFO, NTP_SERVER); // ESP32 Systemzeit mit NTP Synchronisieren
        getLocalTime(&g_bootTime, 10000); 
        ESP_LOGI(TAG, "Time: %2.2d:%2.2d:%2.2d", g_bootTime.tm_hour, g_bootTime.tm_min, g_bootTime.tm_sec);

        //
        // start the remote debugger
        //
	      Debug.begin(g_devicename); // Initialize the WiFi server
        Debug.setResetCmdEnabled(true); // Enable the reset command
	      Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
	      Debug.showColors(false); // Colors
        
        // set to false later
        Debug.setSerialEnabled(true);

        StartModBus(MT_SDM230);
        StartHTTP();
        otaInit();
        StartSensors();
        loraInit();
        dp_drawPage(DP_PAGE_HOME);
    }
}

// housekeeping
uint32_t _minFreeHeap = 0;
uint32_t _freeHeap = 0;

static long _tsMillis = 0;
static long _loopMillis = 0;
static int iWifiRetries = 0;

void loop() 
{
  dp_handle();
  otaHandle();
  ModBusHandle();
  SensorsHandle();
  loraHandle();
  Debug.handle();
 
  // 
  // check every 5 sec for heap size and WIFI connection
  //
  if ((millis() - _tsMillis) > 5000L)
  {
    if (g_minFreeHeap != _minFreeHeap || ESP.getFreeHeap() != _freeHeap) 
    {
      _freeHeap = ESP.getFreeHeap();
      if (_freeHeap < g_minFreeHeap)
        g_minFreeHeap = _freeHeap;
      _minFreeHeap = g_minFreeHeap;

    }
    // Check for connection lost...
    if (WiFi.status() != WL_CONNECTED) 
    {
      ESP_LOGE(TAG, "wifi connection lost");
      WiFi.reconnect();
      iWifiRetries++;

      if (iWifiRetries > 5)      
      {
        ESP_LOGE(TAG, "could not reconnect wifi - restarting??");
        delay(100);
        ESP.restart();
      }
    }
    else
      iWifiRetries = 0;

    _tsMillis = millis();
  }
}
