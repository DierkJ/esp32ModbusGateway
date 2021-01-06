/**
**********************************************************************************************************************************************************************************************************************************
* @file:	i2c.cpp
*
* @brief:	I2C Driver
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
*
**********************************************************************************************************************************************************************************************************************************
**/

#include "globals.h"
#include <Wire.h>
#include "i2c.h"

// Local logging tag
static const char TAG[] = __FILE__;

void i2c_init(void) 
{ 
  Wire.begin(MY_DISPLAY_SDA, MY_DISPLAY_SCL, 400000); 
}

void i2c_deinit(void) 
{
  Wire.~TwoWire(); // shutdown/power off I2C hardware
  // configure pins as input to save power, because Wire.end() enables pullups
  pinMode(MY_DISPLAY_SDA, INPUT);
  pinMode(MY_DISPLAY_SCL, INPUT);
}

int i2c_scan(void) 
{

  int i2c_ret, addr;
  int devices = 0;

  debugD("Starting I2C bus scan...");

  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {

    // Scan at 100KHz low speed
    Wire.setClock(100000);

    for (addr = 8; addr <= 119; addr++) {

      Wire.beginTransmission(addr);
      Wire.write(addr);
      i2c_ret = Wire.endTransmission();

      if (i2c_ret == 0) {
        devices++;

        switch (addr) {

        case SSD1306_PRIMARY_ADDRESS:
        case SSD1306_SECONDARY_ADDRESS:
          debugD("0x%X: SSD1306 Display controller", addr);
          break;

        case BME_PRIMARY_ADDRESS:
        case BME_SECONDARY_ADDRESS:
          debugD("0x%X: Bosch BME MEMS", addr);
          break;

        case AXP192_PRIMARY_ADDRESS:
          debugD("0x%X: AXP192 power management", addr);
          break;

        default:
          debugD("0x%X: Unknown device", addr);
          break;
        }
      } // switch
    }   // for loop

    debugD("I2C scan done, %u devices found.", devices);

    // Set back to 400KHz
    Wire.setClock(400000);

    I2C_MUTEX_UNLOCK(); // release i2c bus access
  } 
  else
    ESP_LOGE(TAG, "I2c bus busy - scan error");

  return devices;
}

// mutexed functions for i2c r/w access
uint8_t i2c_readBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len) 
{
  if (I2C_MUTEX_LOCK()) {

    uint8_t ret = 0;
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    uint8_t cnt = Wire.requestFrom(addr, (uint8_t)len, (uint8_t)1);
    if (!cnt)
      ret = 0xFF;
    uint16_t index = 0;
    while (Wire.available()) {
      if (index > len) {
        ret = 0xFF;
        goto finish;
      }
      data[index++] = Wire.read();
    }

  finish:
    I2C_MUTEX_UNLOCK(); // release i2c bus access
    return ret;
  } 
  else 
  {
    debugD("[%0.3f] i2c mutex lock failed", millis() / 1000.0);
    return 0xFF;
  }
}

uint8_t i2c_writeBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len) 
{
  if (I2C_MUTEX_LOCK()) {

    uint8_t ret = 0;
    Wire.beginTransmission(addr);
    Wire.write(reg);
    for (uint16_t i = 0; i < len; i++) {
      Wire.write(data[i]);
    }
    ret = Wire.endTransmission();

    I2C_MUTEX_UNLOCK(); // release i2c bus access
    return ret ? ret : 0xFF;
  } else 
  {
    debugD("[%0.3f] i2c mutex lock failed", millis() / 1000.0);
    return 0xFF;
  }
}
