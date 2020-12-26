/**
**********************************************************************************************************************************************************************************************************************************
* @file:	ota.h
*
* @brief:	OTA functions
*
*           different methodes for OTA:
*
*           1:  direct OTA per TCP / ESP32 
*           2:  Webupdate from file
*           3:  automatic per Backend
*
* @author:	Dierk Arp
* @date:	20201224 11:36:51 
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
#ifndef _OTA_H_INCLUDED
#define _OTA_H_INCLUDED

#define OTA_DIRECT
// #define OTA_WEB
// #define OTA_BACKEND


void otaInit() ;
void otaHandle() ;


#endif