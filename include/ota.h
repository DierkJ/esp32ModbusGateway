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
#ifndef _OTA_H_INCLUDED
#define _OTA_H_INCLUDED

#define OTA_DIRECT      // use direct upload via TCP
// #define OTA_WEB      // use download from local web page
// #define OTA_IAS         // use IOTAppStory by Andreas Spiess
// #define OTA_BACKEND


void otaInit() ;
void otaHandle() ;


#endif