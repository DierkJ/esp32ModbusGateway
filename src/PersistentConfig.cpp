/**
**********************************************************************************************************************************************************************************************************************************
* @file:	PersistentConfig.cpp
*
* @brief:	Persistent Configuration Data for Modbus Gateway
*
* @author:	Dierk Arp
* @date:	20201129 16:01:24 
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


#include <WiFi.h>
#include "SPIFFS.h"
#include <rom/rtc.h>

#include <MD5Builder.h>
#include <WString.h>
#include <FS.h>
#include <ArduinoJson.h>

#include "modbus.h"
#include "PersistentConfig.h"


PersistentConfig::PersistentConfig()
{
    sMeterType = "SDM630";
}

PersistentConfig::~PersistentConfig()
{
}

/**
 * @brief read json configuration file from SPIFFS
 * 
 * @return true     read successfull
 * @return false    configuration not available
 */
bool PersistentConfig::Load()
{
    ESP_LOGI(TAG, "loading config");
    File configFile = SPIFFS.open(F("/config.json"), "r");
    if (!configFile) 
    {
        ESP_LOGE(TAG, "open config failed");
        return false;
    }

    size_t size = configFile.size();
 
    StaticJsonDocument<512> doc;
    auto error = deserializeJson(doc, configFile);
    if (error) 
    {
       ESP_LOGE(TAG, "deserializeJson() failed with  %s", error.c_str());
        configFile.close();
        return false;
    }
    configFile.close();

    const char *pCC = doc["metertype"];
    ESP_LOGI(TAG, "Config: Metertype: %s", pCC);
    sMeterType = pCC;
    return true;
}

/** 
 * @brief write json configuration file
 * 
 * @return true     write successfull
 * @return false    write not successfull
 */
bool PersistentConfig::Save()
{
  File configFile = SPIFFS.open(F("/config.json"), "w");
  if (!configFile) 
  {
    ESP_LOGE(TAG, "save config failed");
    return false;
  }

  StaticJsonDocument<512> doc;
  doc["metertype"] = sMeterType;

  serializeJson(doc, configFile);
  configFile.close();

  return true;
}
