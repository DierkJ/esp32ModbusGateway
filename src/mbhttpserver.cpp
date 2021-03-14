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

//
// temp index html
//
const char index_html[] PROGMEM = "<!DOCTYPE html>\r\n"
   "<html>\r\n"
   " \r\n"
   "    <head>\r\n"
   "        <script src="
   "\"src/jquery-3.3.1.m"
   "in.js\"></script>\r"
   "\n"
   "        <script>\r\n"
   "            function"
   " hide() {\r\n"
   "                $(\""
   "#paragraph\").hide(2"
   "000);\r\n"
   "            }\r\n"
   "        </script>\r"
   "\n"
   "    </head>\r\n"
   " \r\n"
   "    <body>\r\n"
   " \r\n"
   "        <p id=\"para"
   "graph\">Text to be h"
   "idden</p>\r\n"
   "        <button oncl"
   "ick=\"hide()\">Hide<"
   "/button>\r\n"
   " \r\n"
   "    </body>\r\n"
   "</html>";


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
  debugD("file not found %s", request->url().c_str());
  request->send(404, F(CONTENT_TYPE_PLAIN), F("File not found"));
}

/**
 * Status JSON api
 */
void handleGetStatus(AsyncWebServerRequest *request)
{
  debugD("%s (%d args)", request->url().c_str(), request->params());

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server","Modbus Gateway");
  JsonObject root = response->getRoot();

  if (request->hasParam("initial")) 
  {
    char buf[16];
    snprintf(buf, 16, "%06x", getChipId());
    root[F("cpu")] = "ESP32";
    root[F("serial")] = buf;
    snprintf(buf, 16, "%s", VERSION);
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
  root[F("resetcode")] = getResetReason(0);

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
  debugD("%s (%d args)", request->url().c_str(), request->params());

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server","Modbus Gateway");
  JsonObject root = response->getRoot();

  int iMIdx = 0;
  if (request->hasParam("1"))
    iMIdx = 1;
  if (request->hasParam("2"))
    iMIdx = 1;
  if (request->hasParam("3"))
    iMIdx = 1;

  ModBusMeter *pM = GetMeterDataPtr(iMIdx);
  if (pM)
  {
    root[F("connected")] = pM->isConnected();

    root[F("frequency")] = pM->GetFrequency();
    root[F("energy_out")] = pM->GetEnergyOut();
    root[F("energy_in")] = pM->GetEnergyIn();
    for (int i=0; i<3; i++)
    {
      char szBuf[32];
      sprintf(szBuf, "u_%1.1d", i+1);
      root[szBuf] = pM->GetPhaseVoltage(i);
      sprintf(szBuf, "i_%1.1d", i+1);
      root[szBuf] = pM->GetPhaseCurrent(i);
      sprintf(szBuf, "p_%1.1d", i+1);
      root[szBuf] = pM->GetPhasePower(i);
      sprintf(szBuf, "ap_%1.1d", i+1);
      root[szBuf] = pM->GetApparentPower(i);
      sprintf(szBuf, "rp_%1.1d", i+1);
      root[szBuf] = pM->GetReactivePower(i);
    }
    root[F("cycles")] = (unsigned long)pM->GetCycles();         
    root[F("ErrCnt")] = (unsigned long)pM->GetErrCnt();        
    root[F("DeviceAddr")] = (unsigned long)pM->GetDeviceAddr();   
    root[F("DeviceType")] = pM->GetDeviceType();   
  }
  else
  {
    root[F("connected")] = false;
  }
  g_lastAccessTime = millis();

  response->setLength();
  request->send(response);
}

/**
 * Sensor JSON api
 */
void handleGetSensor(AsyncWebServerRequest *request)
{
  debugD("%s (%d args)", request->url().c_str(), request->params());

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server","Modbus Gateway");
  JsonObject root = response->getRoot();

  root[F("temperature")] = g_SensorData.temperature;
  root[F("pressure")] = g_SensorData.pressure;
  root[F("altitude")] = g_SensorData.altitude;

  g_lastAccessTime = millis();

  response->setLength();
  request->send(response);
}

/**
 * Handle Update.
 */
void handleUpdate(AsyncWebServerRequest *request)
{
  debugD("handleUpdate: %s (%d args)", request->url().c_str(), request->params());

  String resp = F("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"5; url=/\"></head><body>");
  int result = 400;

  if (request->hasParam("Update"))
    debugD("Update");
  else
  {
    debugD("other Button:" );
  }
  resp += F("<h1>Updated.</h1>");
  resp += F("</body></html>");
  request->send(result, F(CONTENT_TYPE_HTML), resp);
}

/**
 * Handle Setup.
 */
void handleSetup(AsyncWebServerRequest *request)
{
  debugD("handleSetup: %s (%d args)", request->url().c_str(), request->params());

  if (request->hasParam("Update"))
    debugD("Update");
  else
  {
    debugD("other Button:" );
  }
}

/**
 * Handle set request from http server.
 */
void handleSettings(AsyncWebServerRequest *request)
{
  debugI("%s (%d args)", request->url().c_str(), request->params());

  String resp = F("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"5; url=/\"></head><body>");
  String ssid = "";
  String pass = "";
  int result = 400;

  if (request->hasParam("metertype", true)) 
  {
    g_cfg.sMeterType = request->getParam("metertype", true)->value();
    result = 200;
  }
  if (result == 400) 
  {
    request->send(result, F(CONTENT_TYPE_PLAIN), F("Bad request\n\n"));
    return;
  }

  if (g_cfg.Save()) 
  {
    resp += F("<h1>Settings saved.</h1>");
  }
  else 
  {
    resp += F("<h1>Failed to save config file.</h1>");
    result = 400;
  }
  resp += F("</body></html>");
  if (result == 200) {
    requestRestart();
  }
  request->send(result, F(CONTENT_TYPE_HTML), resp);
}



const char* PARAM_MESSAGE = "message";

void StartHTTP(void) 
{
  debugD("starting HTTP Server... ");

  //
  // some checks for SPIFFS
  //
  File fp = SPIFFS.open("/index.html", "r");
  if (!fp) 
  {
    debugD("Failed to open index.html for reading, HTTP Server not started");
    return;
  }
  else 
  {
    debugD("index.html Size: %d", fp.size());
  }
  fp.close();

  fp = SPIFFS.open("/src/jquery-3.3.1.min.js", "r");
  if (!fp) 
  {
    debugD("Failed to open /src/jquery-3.3.1.min.js for reading, HTTP Server not started");
    return;
  }
  else 
  {
    debugD("jQuery Size: %d", fp.size());
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


  g_server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    debugD("send index.html");
    request->send_P(200, "text/html", index_html);
    // request->send(SPIFFS, "/index.html", "text/html");
  });


  g_server.on("/src/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    debugD("on jQuery");
    request->send(SPIFFS, "/src/jquery-3.3.1.min.js", "text/javascript");
  }).setFilter(ON_STA_FILTER);


  // GET
  g_server.on("/api/status", HTTP_GET, handleGetStatus);
  g_server.on("/api/meter", HTTP_GET, handleGetPowerMeter);
  g_server.on("/api/sensor", HTTP_GET, handleGetSensor);


  // POST
//  g_server.on("/update", HTTP_POST, handleUpdate );
  g_server.on("/settings", HTTP_POST, handleSettings );


  // make sure config.json is not served!
  g_server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
    request->send(400);
  });

  // catch-all
//  g_server.serveStatic("/", SPIFFS, "/", CACHE_HEADER).setDefaultFile("index.html");

  g_server.begin();
  debugD("HTTP server started... ");

}
