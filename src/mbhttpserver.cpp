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
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

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

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server","ESP Async Web Server");
  JsonObject root = response->getRoot();

  if (request->hasParam("initial")) 
  {
    char buf[16];
    snprintf(buf, 16, "%06x", getChipId());
    root[F("cpu")] = "ESP32";
    root[F("serial")] = buf;
    snprintf(buf, 16, "%s", PROGVERSION);
    root[F("build")] = buf; 
    // root[F("ssid")] = g_ssid;
    // root[F("pass")] = g_pass;
    root[F("flash")] = ESP.getFlashChipSize();
    root[F("wifimode")] = (WiFi.getMode() & WIFI_STA) ? "Connected" : "Access Point";
    root[F("ip")] = getIP();
  }

  long heap = ESP.getFreeHeap();
  root[F("uptime")] = millis();
  root[F("heap")] = heap;
  root[F("minheap")] = g_minFreeHeap;
  root[F("lastaccess")] = g_lastAccessTime;
//  root[F("resetcode")] = getResetReason(0);
//  root[F("gpio")] = (uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16));

  // reset free heap
  g_minFreeHeap = heap;
  g_lastAccessTime = millis();

  response->setLength();
  request->send(response);
}

/**
 * PowerMeter JSON api
 */
void handleGetPowerMeter(AsyncWebServerRequest *request)
{
  ESP_LOGI(TAG, "%s (%d args)", request->url().c_str(), request->params());

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server","ESP Async Web Server");
  JsonObject root = response->getRoot();

  root[F("connected")] = g_modBusMeterData.fConnected;

  root[F("voltage")] = g_modBusMeterData.fVoltage;
  root[F("current")] = g_modBusMeterData.fCurrent;
  root[F("power")] = g_modBusMeterData.fPower;
  root[F("reactive_power")] = g_modBusMeterData.fReactivePower;
  root[F("frequency")] = g_modBusMeterData.fFrequency;
  root[F("energy_out")] = g_modBusMeterData.fEnergyOut;
  root[F("energy_in")] = g_modBusMeterData.fEnergyIn;
  for (int i=0; i<3; i++)
  {
    char szBuf[32];
    sprintf(szBuf, "u_phase_%1.1d", i+1);
    root[szBuf] = g_modBusMeterData.fPhaseVoltage[i];
    sprintf(szBuf, "i_phase_%1.1d", i+1);
    root[szBuf] = g_modBusMeterData.fPhaseCurrent[i];
  }
  root[F("cycles")] = g_modBusMeterData.iCycles;         
  root[F("ErrCnt")] = g_modBusMeterData.iErrCnt;        

  g_lastAccessTime = millis();

  response->setLength();
  request->send(response);
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

const char* PARAM_MESSAGE = "message";

void StartHTTP(void) 
{
  ESP_LOGI(TAG, "starting HTTP Server... ");

  //
  // some checks for SPIFFS
  //
  File fp = SPIFFS.open("/index.html", "r");
  if (!fp) 
  {
    ESP_LOGI(TAG, "Failed to open index.html for reading, HTTP Server not started");
    return;
  }
  else 
  {
    ESP_LOGI(TAG, "index.html Size: %d", fp.size());
  }
  fp.close();

  fp = SPIFFS.open("/src/jquery-3.3.1.min.js", "r");
  if (!fp) 
  {
    ESP_LOGI(TAG, "Failed to open /src/jquery-3.3.1.min.js for reading, HTTP Server not started");
    return;
  }
  else 
  {
    ESP_LOGI(TAG, "jQuery Size: %d", fp.size());
  }
  fp.close();

  // register not found
  g_server.onNotFound(handleNotFound);
  
/*
  g_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    // Send a GET request to <IP>/get?message=<message>
  g_server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
  g_server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });
*/


  g_server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });


  g_server.on("/src/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    ESP_LOGI(TAG, "on jQuery");
    request->send(SPIFFS, "/src/jquery-3.3.1.min.js", "text/javascript");
  });


  // GET
  g_server.on("/api/status", HTTP_GET, handleGetStatus);
  g_server.on("/api/meter", HTTP_GET, handleGetPowerMeter);

/*
  // POST
  g_server.on("/update", HTTP_POST, handleUpdate );
  g_server.on("/setup", HTTP_POST, handleSetup );
*/

  // make sure config.json is not served!
  g_server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
    request->send(400);
  });

/*
  // catch-all
  g_server.serveStatic("/", SPIFFS, "/", CACHE_HEADER).setDefaultFile("index.html");
*/


  g_server.begin();
  ESP_LOGI(TAG, "HTTP server started... ");

}
