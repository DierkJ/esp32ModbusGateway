/**
**********************************************************************************************************************************************************************************************************************************
* @file:	modbus.h
*
* @brief:	read modbus meter
* @author:	Dierk Arp
* @date:	20201129 16:01:24 
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
#ifndef _MODBUS_H_INCLUDED
#define _MODBUS_H_INCLUDED

//
// modbus meter data
//
typedef struct {
  
  // values
  float fVoltage;           // voltage [V]
  float fCurrent;           // Current [A] 
  float fPower;             // Power [W]
  float fReactivePower;     // reactive Power [VAr] 
  float fFrequency;         // line frequency [Hz]
  float fEnergyOut;         // el. energy consumption [kWh]
  float fEnergyIn;          // el. energy production  [kWh]
  
  // only for 3 phase meters (SDM 630)
  float fPhaseVoltage[3];   // voltages on phase 0,1,2 [V]
  float fPhaseCurrent[3];   // current on phase 0,1,2 [A]
  
  // communication status 
  boolean fConnected;       // are we connected
  uint32_t iCycles;         // # of read cycles
  uint16_t iErrCnt;         // # communication errors
  uint16_t iLastErr;        // # of last error
} modbus_meter_t;

extern modbus_meter_t g_modBusMeterData;

typedef enum {
    MT_SDM630,
    MT_SDM230,
    MT_SDM220, 
    MT_SDM120,
    MT_SDM72D, 
    MT_DDM,
    MT_FINDER  
} modbus_meter_type_t;

typedef struct {
    modbus_meter_type_t eDeviceType;    // type of device 
    uint16_t iDeviceAddr;               // address on Modbus
    uint32_t iBaudrate;                 // baudrate
} modbus_config_t;

extern const char *Device2Text(modbus_meter_type_t mt);
extern void StartModBus(modbus_meter_type_t dt = MT_SDM630, uint16_t devadr = 1, uint32_t baudrate = 9600);
extern void ModBusHandle(void);


#endif

