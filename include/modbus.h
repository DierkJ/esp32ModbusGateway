/**
**********************************************************************************************************************************************************************************************************************************
* @file:	modbus.h
*
* @brief:	read modbus meter
* @author:	Dierk Arp
* @date:	20201129 16:01:24 
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

extern void StartModBus(modbus_meter_type_t dt = MT_SDM630, uint16_t devadr = 1, uint32_t baudrate = 9600);
extern void ModBusHandle(void);


#endif

