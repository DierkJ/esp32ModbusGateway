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

