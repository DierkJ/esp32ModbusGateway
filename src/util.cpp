/**
**********************************************************************************************************************************************************************************************************************************
* @file:	util.cpp
*
* @brief:	some stupid functions...
*
* @author:	Dierk Arp
* @date:	20201129 09:52:12 Monday
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

#include "globals.h"
#include "util.h"

// Local logging tag
static const char TAG[] = __FILE__;

uint32_t g_minFreeHeap = -1;

long getChipId()
{
  long chipId;
  ESP.getEfuseMac();
  return chipId;
}

String getUptimeString() 
{
  uint16_t days;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;

  #define SECS_PER_MIN  60
  #define SECS_PER_HOUR 3600
  #define SECS_PER_DAY  86400

  time_t uptime = millis() / 1000;

  seconds = uptime % SECS_PER_MIN;
  uptime -= seconds;
  minutes = (uptime % SECS_PER_HOUR) / SECS_PER_MIN;
  uptime -= minutes * SECS_PER_MIN;
  hours = (uptime % SECS_PER_DAY) / SECS_PER_HOUR;
  uptime -= hours * SECS_PER_HOUR;
  days = uptime / SECS_PER_DAY;

  char buffer[20];
  sprintf(buffer, "%4u days %02d:%02d:%02d", days, hours, minutes, seconds);
  return buffer;  
}

