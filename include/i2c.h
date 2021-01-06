/**
**********************************************************************************************************************************************************************************************************************************
* @file:	i2c.h
*
* @brief:	I2C Driver
*
* @author:	Dierk Arp
* @date:	20201129 09:51:34 Monday
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

#ifndef _I2C_H
#define _I2C_H

#define SSD1306_PRIMARY_ADDRESS (0x3D)
#define SSD1306_SECONDARY_ADDRESS (0x3C)
#define BME_PRIMARY_ADDRESS (0x77)
#define BME_SECONDARY_ADDRESS (0x76)
#define AXP192_PRIMARY_ADDRESS (0x34)

#ifndef MY_DISPLAY_SDA
#define MY_DISPLAY_SDA SDA
#endif

#ifndef MY_DISPLAY_SCL
#define MY_DISPLAY_SCL SCL
#endif

void i2c_init(void);
void i2c_deinit(void);
int i2c_scan(void);
uint8_t i2c_readBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);
uint8_t i2c_writeBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);

#endif