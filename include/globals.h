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


// application includes
#include "util.h"
#include "display.h"
#include "modbus.h"
#include "ota.h"
#include "sensors.h"

#endif

