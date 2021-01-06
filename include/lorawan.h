/**
**********************************************************************************************************************************************************************************************************************************
* @file:	lorawan.h
*
* @brief:	lorawan functions
*
*
* @author:	Dierk Arp
* @date:	20201230 11:36:51 
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

**********************************************************************************************************************************************************************************************************************************
**/
#ifndef _LORAWAN_H_INCLUDED
#define _LORAWAN_H_INCLUDED

typedef struct {
    boolean fJoined;    // Joined TTN

    uint32_t netid;         // session parameter
    uint32_t devaddr;
    uint8_t nwkKey[16];
    uint8_t artKey[16];

    uint32_t nTX;       // number of messages transmitted
    uint32_t nAck;      // number of transmitted messages with acknowledge
    uint32_t nRX;       // number of messages received
    uint16_t rssi;      // signal strength?
} lora_status_t;

extern lora_status_t g_LoraData;


void loraInit() ;
void loraHandle() ;


#endif