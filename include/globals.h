/**
**********************************************************************************************************************************************************************************************************************************
* @file:	globals.h
*
* @brief:	master include
*
* @author:	Dierk Arp
* @date:	20201129 11:40:51 Sunday
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
#ifndef _GLOBALS_H
#define _GLOBALS_H

// The mother of all embedded development...
#include <Arduino.h>
#include "ttgov1.h"   // our board

#define COMPDATE __DATE__ __TIME__

// config defines
#define OTA_SERVER
// #define CAPTIVE_PORTAL
// #define SPIFFS_EDITOR
#define HAS_LORA

// #define WIFI_MANAGER
#define WIFI_CONNECT_TIMEOUT  (15*10)    // 15 sec to connect

// Remote debugging / logging
#define WEBSOCKET_DISABLED      // only TELNET
#include "RemoteDebug.h"        //
extern RemoteDebug Debug;


#define PANIC(...) abort()

// I2C bus access control
#define I2C_MUTEX_LOCK()                                                       \
  (xSemaphoreTake(I2Caccess, pdMS_TO_TICKS(DISPLAYREFRESH_MS)) == pdTRUE)
#define I2C_MUTEX_UNLOCK() (xSemaphoreGive(I2Caccess))

enum sendprio_t { prio_low, prio_normal, prio_high };
enum timesource_t { _gps, _rtc, _lora, _unsynced };

enum runmode_t {
  RUNMODE_POWERCYCLE = 0,
  RUNMODE_NORMAL,
  RUNMODE_WAKEUP,
  RUNMODE_UPDATE
};

// Struct holding devices's runtime configuration
typedef struct {
  uint8_t loradr;      // 0-15, lora datarate
  uint8_t txpower;     // 2-15, lora tx power
  uint8_t adrmode;     // 0=disabled, 1=enabled
  uint8_t screensaver; // 0=disabled, 1=enabled
  uint8_t screenon;    // 0=disabled, 1=enabled
  uint8_t countermode; // 0=cyclic unconfirmed, 1=cumulative, 2=cyclic confirmed
  int16_t rssilimit;   // threshold for rssilimiter, negative value!
  uint8_t sendcycle;   // payload send cycle [seconds/2]
  uint8_t runmode;     // 0=normal, 1=update
  uint8_t payloadmask; // bitswitches for payload data
  char version[10];    // Firmware version
} configData_t;

extern SemaphoreHandle_t I2Caccess;
extern String g_devicename ;
extern String g_ipAddress ;
extern String g_ipSubNet ;
extern String g_SSID ;


// application includes
#include "util.h"
#include "display.h"
#include "modbus.h"
#include "ota.h"
#include "lorawan.h"
#include "sensors.h"
#include "PersistentConfig.h"

extern PersistentConfig g_cfg;



#endif

