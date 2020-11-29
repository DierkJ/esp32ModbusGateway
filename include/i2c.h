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