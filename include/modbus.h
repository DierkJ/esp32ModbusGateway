/**
**********************************************************************************************************************************************************************************************************************************
* @file:	modbus.h
*
* @brief:	read modbus meter
* @author:	Dierk Arp
* @date:	20201129 16:01:24 
* @version:	2.0
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

#include "ModbusClientRTU.h"

/// defines the different supported Modbus meter types 
enum eMeterType
{
      MT_SDM630,
      MT_SDM230,
      MT_SDM220, 
      MT_SDM120,
      MT_SDM72D, 
      MT_DDM,
      MT_FINDER,
      MT_UNKNOWN
};


class ModBusMeter {

protected:

public : 
    ModBusMeter();
    ~ModBusMeter();

    // modbus handler
    void handleMeterData(ModbusMessage response, uint32_t token);
    void handleMeterError(Error error, uint32_t token);
    Error FireConnectRequest(void);
    void FireDataRequest(void);

    // access functions
    void SetMeter(eMeterType mt = MT_SDM630, int iDevAddr = 1);

    float GetPhaseVoltage(int iPhase)   { return fPhaseVoltage[iPhase & 3]; }
    float GetPhaseCurrent(int iPhase)   { return fPhaseCurrent[iPhase & 3]; }
    float GetPhasePower(int iPhase)     { return fPhasePower[iPhase & 3]; }
    float GetApparentPower(int iPhase)  { return fApparentPower[iPhase & 3]; } 
    float GetReactivePower(int iPhase)  { return fReactivePower[iPhase & 3]; }
  
    float GetFrequency()  { return fFrequency; }  
    float GetEnergyOut()  { return fEnergyOut; }
    float GetEnergyIn()   { return fEnergyIn; }
  
    boolean isConnected() { return fConnected; } 
    uint32_t GetCycles()  { return iCycles; }
    uint16_t GetErrCnt()  { return iErrCnt; }         
    uint16_t GetLastErr() { return iLastErr; }   
    
    uint16_t GetDeviceAddr()  { return iDeviceAddr; }     
    String GetDeviceType()    { return MeterType2Text(eDeviceType); }

// helper member

    const char *MeterType2Text(eMeterType mt);
    eMeterType Text2MeterType(const String&  sDevText);

private:
    float toFloat(ModbusMessage response);
    uint16_t toInt16(ModbusMessage response);
    uint32_t toInt32(ModbusMessage response);


    //
    // modbus meter data
    //
    // only for 3 phase meters (SDM 630)  all phases are read
    float fPhaseVoltage[3];   // voltages on phase        [V]
    float fPhaseCurrent[3];   // current on phase         [A]
    float fPhasePower[3];     // power on phase           [A]
    float fApparentPower[3];  // apparent Power on phase  [VA] 
    float fReactivePower[3];  // reactive Power on phase  [VAr] 
  
    float fFrequency;         // line frequency           [Hz]
    float fEnergyOut;         // el. energy consumption   [kWh]
    float fEnergyIn;          // el. energy production    [kWh]
  
    // communication status 
    boolean fConnected;       // are we connected
    uint32_t iCycles;         // # of read cycles
    uint16_t iErrCnt;         // # communication errors
    uint16_t iLastErr;        // # of last error
    uint16_t uFWVersion;      // firmware version register (SDM?  todo)

    // 
    eMeterType eDeviceType;    // type of device 
    uint16_t iDeviceAddr;      // address on Modbus

};


extern void StartModBus(uint32_t baudrate, int iNMeters, eMeterType *dt, uint16_t *devadr);
extern void ModBusHandle(void);
extern ModBusMeter *GetMeterDataPtr(int idx);
extern int GetNumberOfMeters(void);


#endif

