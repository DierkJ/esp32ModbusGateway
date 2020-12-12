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
#include "ESPAsyncWebServer.h"

#define CACHE_HEADER "max-age=86400"
#define CONTENT_TYPE_JSON "application/json"
#define CONTENT_TYPE_PLAIN "text/plain"
#define CONTENT_TYPE_HTML "text/html"



AsyncWebServer g_server(80);

// We declare a function that will be the entry-point for the task that is going to be
// created.
void serverTask(void *params);

/**
 * Handle set request from http server.
 */
void handleNotFound(AsyncWebServerRequest *request)
{
  ESP_LOGI(TAG, "file not found %s\n", request->url().c_str());
  request->send(404, F(CONTENT_TYPE_PLAIN), F("File not found"));
}


void StartHTTP(void) 
{
  // Open SPI FF
  if(!SPIFFS.begin())
  {
     ESP_LOGI(TAG, "An Error has occurred while mounting SPIFFS");
     return;
  }

  // Setup the server as a separate task.
  //ESP_LOGI(TAG, "Creating server task... ");
  // We pass:
  // serverTask   - the function that should be run as separate task
  // "httpserver" - a name for the task (mainly used for logging)
  // 6144         - stack size in byte. If you want up to four clients, you should
  //              not go below 6kB. If your stack is too small, you will encounter
  //              Panic and stack canary exceptions, usually during the call to
  //              SSL_accept.
  
   //xTaskCreatePinnedToCore(serverTask, "httpserver", 6144, NULL, 1, NULL, 1);

  // not found
  g_server.onNotFound(handleNotFound);

  g_server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
 
  g_server.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/bootstrap.bundle.min.js", "text/javascript");
  });
 
  g_server.on("/src/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/jquery-3.3.1.min.js", "text/javascript");
  });
 
  g_server.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/bootstrap.min.css", "text/css");
  });
 
  // catch-all
  g_server.serveStatic("/", SPIFFS, "/", CACHE_HEADER).setDefaultFile("index.html");


  g_server.begin();
  ESP_LOGI(TAG, "Bootstrap server started... ");

}


void serverTask(void *params) 
{
  // In the separate task we first do everything that we would have done in the
  // setup() function, if we would run the server synchronously.

  // Note: The second task has its own stack, so you need to think about where
  // you create the server's resources and how to make sure that the server
  // can access everything it needs to access. Also make sure that concurrent
  // access is no problem in your sketch or implement countermeasures like locks
  // or mutexes.
  g_server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
 
  g_server.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/bootstrap.bundle.min.js", "text/javascript");
  });
 
  g_server.on("/src/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/jquery-3.3.1.min.js", "text/javascript");
  });
 
  g_server.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/src/bootstrap.min.css", "text/css");
  });
 
  g_server.begin();
  ESP_LOGI(TAG, "Bootstrap server started... ");
  
}
  