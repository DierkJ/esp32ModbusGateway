/**
**********************************************************************************************************************************************************************************************************************************
* @file:	main.h
*
* @brief:	Main Header
*
* @author:	Dierk Arp
* @date:	20201129 11:45:15 Sunday
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
#ifndef _MAIN_H_INCLUDED
#define _MAIN_H_INCLUDED

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

#include <esp_spi_flash.h>   // needed for reading ESP32 chip attributes
#include <esp_event_loop.h>  // needed for Wifi event handler
#include <esp32-hal-timer.h> // needed for timers
#include <esp_coexist.h>     // needed for showing coex sw version


#include "globals.h"

#include <WiFi.h>
#include <WiFiManager.h> 

#endif