/**
**********************************************************************************************************************************************************************************************************************************
* @file:	mbhttpserver.cpp
*
* @brief:	http Server for Modbus Gateway
*
* @author:	Dierk Arp
* @date:	20201129 16:01:24 
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

#include "globals.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>

#ifdef SPIFFS_EDITOR
#include "SPIFFSEditor.h"
#endif


#define CACHE_HEADER "max-age=86400"
#define CORS_HEADER "Access-Control-Allow-Origin"

#define CONTENT_TYPE_JSON "application/json"
#define CONTENT_TYPE_PLAIN "text/plain"
#define CONTENT_TYPE_HTML "text/html"


uint32_t g_restartTime = 0;
uint32_t g_lastAccessTime = 0;

AsyncWebServer g_server(80);

void requestRestart()
{
  g_restartTime = millis() + 100;
}

void jsonResponse(AsyncWebServerRequest *request, int res, JsonVariant json)
{
  // touch
  g_lastAccessTime = millis();

  AsyncResponseStream *response = request->beginResponseStream(F(CONTENT_TYPE_JSON));
  response->addHeader(F(CORS_HEADER), "*");
  serializeJson(json, *response);
  request->send(response);
}

String getIP()
{
  IPAddress ip = (WiFi.getMode() & WIFI_STA) ? WiFi.localIP() : WiFi.softAPIP();
  return ip.toString();
}

/**
 * Handle set request from http server.
 */
void handleNotFound(AsyncWebServerRequest *request)
{
  ESP_LOGI(TAG, "file not found %s", request->url().c_str());
  request->send(404, F(CONTENT_TYPE_PLAIN), F("File not found"));
}

/**
 * Status JSON api
 */
void handleGetStatus(AsyncWebServerRequest *request)
{
  ESP_LOGI(TAG, "%s (%d args)", request->url().c_str(), request->params());

  StaticJsonDocument<512> jsonDoc;
  JsonObject json = jsonDoc.as<JsonObject>(); 

  if (request->hasParam("initial")) 
  {
    char buf[16];
    snprintf(buf, 16, "%06x", getChipId());
    json[F("cpu")] = "ESP32";
    json[F("serial")] = buf;
    snprintf(buf, 16, "%s", PROGVERSION);
    json[F("build")] = buf; 
    // json[F("ssid")] = g_ssid;
    // json[F("pass")] = g_pass;
    json[F("flash")] = ESP.getFlashChipSize();
    json[F("wifimode")] = (WiFi.getMode() & WIFI_STA) ? "Connected" : "Access Point";
    json[F("ip")] = getIP();
  }

  long heap = ESP.getFreeHeap();
  json[F("uptime")] = millis();
  json[F("heap")] = heap;
  json[F("minheap")] = g_minFreeHeap;
//  json[F("resetcode")] = getResetReason(0);
  // json[F("gpio")] = (uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16));

  // reset free heap
  g_minFreeHeap = heap;

  jsonResponse(request, 200, json);
}

/**
 * PowerMeter JSON api
 */
void handleGetPowerMeter(AsyncWebServerRequest *request)
{
  ESP_LOGI(TAG, "%s (%d args)", request->url().c_str(), request->params());

  StaticJsonDocument<512> jsonDoc;
  JsonObject json = jsonDoc.as<JsonObject>(); 

  json[F("connected")] = g_modBusMeterData.fConnected;

  json[F("voltage")] = g_modBusMeterData.fVoltage;
  json[F("current")] = g_modBusMeterData.fCurrent;
  json[F("power")] = g_modBusMeterData.fPower;
  json[F("frequency")] = g_modBusMeterData.fFrequency;
  json[F("energyout")] = g_modBusMeterData.fEnergyOut;
  json[F("energyin")] = g_modBusMeterData.fEnergyIn;
  for (int i=0; i<3; i++)
  {
    char szBuf[32];
    sprintf(szBuf, "u_phase_%1.1d", i+1);
    json[szBuf] = g_modBusMeterData.fPhaseVoltage[i];
    sprintf(szBuf, "i_phase_%1.1d", i+1);
    json[szBuf] = g_modBusMeterData.fPhaseCurrent[i];
  }
  json[F("cycles")] = g_modBusMeterData.iCycles;         
  json[F("ErrCnt")] = g_modBusMeterData.iErrCnt;        

  jsonResponse(request, 200, json);
}


/**
 * Handle Update.
 */
void handleUpdate(AsyncWebServerRequest *request)
{
  ESP_LOGI(TAG, "handleUpdate: %s (%d args)", request->url().c_str(), request->params());

  String resp = F("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"5; url=/\"></head><body>");
  int result = 400;

  if (request->hasParam("Update"))
    ESP_LOGI(TAG, "Update");
  else
  {
    ESP_LOGI(TAG, "other Button:" );
  }
  resp += F("<h1>Updateed.</h1>");
  resp += F("</body></html>");
  request->send(result, F(CONTENT_TYPE_HTML), resp);
}

/**
 * Handle Setup.
 */
void handleSetup(AsyncWebServerRequest *request)
{
    ESP_LOGI(TAG, "handleSetup: %s (%d args)", request->url().c_str(), request->params());

  if (request->hasParam("Update"))
    ESP_LOGI(TAG, "Update");
  else
  {
    ESP_LOGI(TAG, "other Button:" );
  }
}



void StartHTTP(void) 
{

  // register not found
  g_server.onNotFound(handleNotFound);

  g_server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  

  g_server.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/src/bootstrap.bundle.min.js", "text/javascript");
  });
 
  g_server.on("/src/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/jquery-3.3.1.min.js", "text/javascript");
  });
 
  g_server.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/bootstrap.min.css", "text/css");
  });
 

  // GET
  g_server.on("/api/status", HTTP_GET, handleGetStatus);
  g_server.on("/api/meter", HTTP_GET, handleGetPowerMeter);

  // POST
  g_server.on("/update", HTTP_POST, handleUpdate );
  g_server.on("/setup", HTTP_POST, handleSetup );

  // make sure config.json is not served!
  g_server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(400);
  });

  // catch-all
  g_server.serveStatic("/", SPIFFS, "/", CACHE_HEADER).setDefaultFile("index.html");

  g_server.begin();
  ESP_LOGI(TAG, "Bootstrap server started... ");

}
