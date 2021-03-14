/**
**********************************************************************************************************************************************************************************************************************************
* @file:	modbus.cpp
*
* @brief:	read modbus task
*
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
static const char TAG[] = __FILE__;

#include "globals.h"

#include "modbus.h"
#include "ModbusRegister.h"
#include "logging.h"


#define MAX_METERS  (4)
ModBusMeter ModMeters[MAX_METERS];
int iNMeters = 0;

//pins for Serial2 => RX pin 35, TX pin 13, pin 17: RTS (Rx/Tx switch)
ModbusClientRTU MB(Serial2, 17); 

#define MODBUSCYCLE          (10)       // read meter data every x s

// eModBus Token usage:
// token & 0xffff   : lower 16bit for register/command id
// token >> 16      :  bits 16..18: for device type
// token >> 24      :  bit 24,25: for device idx
// some special tokens..
#define TOK_START   (0x4711)          // start identifier of a cycle
#define TOK_FINAL   (0x10000000L)     // last command of a cycle


//
// ModBus Meter Class
//
ModBusMeter::ModBusMeter()
{
    fConnected = false; 
    iCycles = 0;
    iErrCnt = 0; 
}

ModBusMeter::~ModBusMeter()
{
}

void ModBusMeter::SetMeter(eMeterType mt, int iDevAddr)
{
    eDeviceType = mt;
    iDeviceAddr = iDevAddr;
}

//
// device decoder
//
const char *ModBusMeter::MeterType2Text(eMeterType mt)
{
    switch (mt)
    {
        case MT_SDM630:
            return "SDM630";
            break;
        case MT_SDM230:
            return "SDM230";
            break;
        case MT_SDM220:
            return "SDM220";
            break;
        case MT_SDM120:
            return "SDM120";
            break;
        case MT_SDM72D:
            return "SDM72D";
            break;
        case MT_DDM:
            return "DDM";
            break;
        case MT_FINDER:
            return "FINDER";
            break;
        default:
            return "unknown";
            break;
    }
}

eMeterType ModBusMeter::Text2MeterType(const String&  sDevText)
{
    eMeterType dt = MT_UNKNOWN;

    if (sDevText.length() > 0)
    {   
        String sTmp = sDevText;
        sTmp.trim();
        if (sTmp.equalsIgnoreCase(F("SDM")))
        {
            int iDev = sTmp.substring(3, 2).toInt();
            switch (iDev)
            {
                case 63:
                    dt = MT_SDM630;
                    break;
                case 23:
                    dt = MT_SDM230;
                    break;
                case 22:
                    dt = MT_SDM220;
                    break;
                case 12:
                    dt = MT_SDM220;
                    break;
                case 72:
                    dt = MT_SDM72D;
                    break;
            }
        }
        else if (sTmp.equalsIgnoreCase(F("DDM")))
            dt = MT_DDM;
        else if (sTmp.equalsIgnoreCase(F("FINDER")))
            dt = MT_FINDER;
    }
    return dt;
}

float ModBusMeter::toFloat(ModbusMessage response)
{
    float fTmp = NAN;

    ((uint8_t*)&fTmp)[3]= response[3];
    ((uint8_t*)&fTmp)[2]= response[4];
    ((uint8_t*)&fTmp)[1]= response[5];
    ((uint8_t*)&fTmp)[0]= response[6];

    if (!isnan(fTmp))
    {
        debugV("Modbus float: %f", fTmp);
        return fTmp;
    }
    else
        return 0.0;
}

uint16_t ModBusMeter::toInt16(ModbusMessage response)
{
    uint16_t uTmp;
  
    ((uint8_t*)&uTmp)[1]= response[3];
    ((uint8_t*)&uTmp)[0]= response[4];
    return uTmp;
}

uint32_t ModBusMeter::toInt32(ModbusMessage response)
{
    uint32_t ulTmp;
  
    ((uint8_t*)&ulTmp)[3]= response[3];
    ((uint8_t*)&ulTmp)[2]= response[4];
    ((uint8_t*)&ulTmp)[1]= response[5];
    ((uint8_t*)&ulTmp)[0]= response[6];
    return ulTmp;
}

/**
 * @brief onData handler function to receive the regular responses
 * 
 * @param response  Modbus server ID, the function code requested, the message data and length of it
 * @param token     user-supplied token to identify the causing request
 */
void ModBusMeter::handleMeterData(ModbusMessage response, uint32_t token) 
{
    debugV("Response: serverID=%d, FC=%d, Token=%08X, length=%d:", response.getServerID(), response.getFunctionCode(), token, response.size());

#if (0)
    for (auto& byte : response) 
    {
        debugD("%02X ", byte);
    }
#endif

    if (token == TOK_START)
        fConnected = true;
    else
    {
        eMeterType mt = (eMeterType) ((token >> 16) - 1);
        if (mt == MT_FINDER)
        {
            switch (token & 0xffffL)
            {
                case FINDER_FIRMWARE_VERSION:
                    uFWVersion = toInt16(response);
                    debugI("Finder firmware: %d.%d", uFWVersion/10, uFWVersion%10);
                    break;
                case FINDER_PHASE_1_VOLTAGE:
                    fPhaseVoltage[0] = (float) toInt16(response);
                    break;
                case FINDER_PHASE_1_CURRENT:
                    fPhaseCurrent[0] = (float)toInt16(response) * 0.1;
                     break;
                case FINDER_PHASE_1_POWER:
                    fPhasePower[0] = (float)toInt16(response) * 0.01;
                    break;
                 case FINDER_PHASE_1_REACTIVE_POWER:
                    fReactivePower[0] = (float)toInt16(response) * 0.01;
                    break;
                 case FINDER_IMPORT_ACTIVE_ENERGY:
                    fEnergyIn = (float)toInt32(response) * 0.01;
                    break;
                 case FINDER_EXPORT_ACTIVE_ENERGY:
                    fEnergyOut = (float)toInt32(response) * 0.01;
                    break;
            }
        }
        else if (mt == MT_DDM)
        {
            debugD("DDM not yet supported");
        }
        else    // mt == all Finder
        {
            switch (token & 0xffffL)
            {
                case SDM_FREQUENCY: 
                    fFrequency = toFloat( response);
                    break;
                case SDM_IMPORT_ACTIVE_ENERGY: 
                    fEnergyIn = toFloat( response);
                    break;
                case SDM_EXPORT_ACTIVE_ENERGY: 
                    fEnergyOut = toFloat( response);
                    break;
                case SDM_PHASE_1_VOLTAGE:
                    fPhaseVoltage[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_VOLTAGE:
                    fPhaseVoltage[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_VOLTAGE:
                    fPhaseVoltage[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_CURRENT:
                    fPhaseCurrent[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_CURRENT:
                    fPhaseCurrent[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_CURRENT:
                    fPhaseCurrent[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_POWER:
                    fPhasePower[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_POWER:
                    fPhasePower[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_POWER:
                    fPhasePower[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_APPARENT_POWER:
                    fApparentPower[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_APPARENT_POWER:
                    fApparentPower[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_APPARENT_POWER:
                    fApparentPower[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_REACTIVE_POWER:
                    fReactivePower[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_REACTIVE_POWER:
                    fReactivePower[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_REACTIVE_POWER:
                    fReactivePower[2] = toFloat( response);
                    break;
            }
        }
        if (token & TOK_FINAL)
        {
            iCycles++;
        }
    }
}

/**
 * @brief Define an onError handler function to receive error responses
 * 
 * @param error error code
 * @param token user-supplied token to identify the causing request
 */
void ModBusMeter::handleMeterError(Error error, uint32_t token) 
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  debugD("Error response: %02X - %s", (int)me, (const char *)me);
  iErrCnt++;
  iLastErr = error;
  
  // reset connected status
  fConnected = false;
}

Error ModBusMeter::FireConnectRequest(void)
{
    uint32_t uStartToken = TOK_START;

    if (eDeviceType == MT_FINDER)
    {   
        // start with Firmware holding register
        return  MB.addRequest(uStartToken, iDeviceAddr, READ_HOLD_REGISTER, FINDER_FIRMWARE_VERSION, 1);
    }
    else if (eDeviceType == MT_DDM)
        return  MB.addRequest(uStartToken, iDeviceAddr, READ_INPUT_REGISTER, DDM_PHASE_1_VOLTAGE, 2);
    else
        return  MB.addRequest(uStartToken, iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_VOLTAGE, 2);
}


void ModBusMeter::FireDataRequest(void)
{
    if (fConnected)
    {
        //
        // put all requests in queue
        // code device type and register into token
        //
        uint32_t uT = ((uint32_t)eDeviceType + 1) << 16;

        if (eDeviceType == MT_FINDER)
        {
            MB.addRequest(uT + FINDER_PHASE_1_VOLTAGE , iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_VOLTAGE, 1);
            MB.addRequest(uT + FINDER_PHASE_1_CURRENT , iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_CURRENT, 1);
            MB.addRequest(uT + FINDER_PHASE_1_POWER , iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_POWER, 1);
            MB.addRequest(uT + FINDER_PHASE_1_REACTIVE_POWER , iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_REACTIVE_POWER, 1);
            MB.addRequest(uT + FINDER_IMPORT_ACTIVE_ENERGY , iDeviceAddr, READ_HOLD_REGISTER, FINDER_IMPORT_ACTIVE_ENERGY, 2);
            // mark last request in cycle:
            uT |= TOK_FINAL;                
            MB.addRequest(uT + FINDER_EXPORT_ACTIVE_ENERGY , iDeviceAddr, READ_HOLD_REGISTER, FINDER_EXPORT_ACTIVE_ENERGY, 2);
        }
        else if (eDeviceType == MT_DDM)
        {
            debugD("DDM not yet supported");
        }
        else
        {
            int i;
            for (i=0; i<3; i++)
            {
                // 3 phase phase meters (SDM 630) scan all or only phase 1
                if ( (eDeviceType == MT_SDM630) || (i<1) )
                {
                    MB.addRequest(uT + SDM_PHASE_1_VOLTAGE + 2*i, iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_VOLTAGE + 2*i, 2);
                    MB.addRequest(uT + SDM_PHASE_1_CURRENT + 2*i, iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_CURRENT + 2*i, 2);
                    MB.addRequest(uT + SDM_PHASE_1_POWER + 2*i, iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_POWER + 2*i, 2);
                    MB.addRequest(uT + SDM_PHASE_1_APPARENT_POWER + 2*i, iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_APPARENT_POWER + 2*i, 2);
                    MB.addRequest(uT + SDM_PHASE_1_REACTIVE_POWER + 2*i, iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_REACTIVE_POWER + 2*i, 2);
                }
            }
            MB.addRequest(uT + SDM_FREQUENCY, iDeviceAddr, READ_INPUT_REGISTER, SDM_FREQUENCY, 2);
            MB.addRequest(uT + SDM_IMPORT_ACTIVE_ENERGY, iDeviceAddr, READ_INPUT_REGISTER, SDM_IMPORT_ACTIVE_ENERGY, 2);
            // mark last request in cycle:
            uT |= TOK_FINAL;
            MB.addRequest(uT + SDM_EXPORT_ACTIVE_ENERGY, iDeviceAddr, READ_INPUT_REGISTER, SDM_EXPORT_ACTIVE_ENERGY, 2);
        }
    }
    else
    {
        // retry to connect...
        Error err = FireConnectRequest();
        if (err != SUCCESS) 
        {
            ModbusError e(err);
            debugD("Error creating request: %02X - %s", (int)e, (const char *)e);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void handleData(ModbusMessage response, uint32_t token)
{
    debugV("Response: serverID=%d, FC=%d, Token=%08X, length=%d:", response.getServerID(), response.getFunctionCode(), token, response.size());
    
    // get meter instance from server id
    for (int i = 0; i<iNMeters; i++)
    {
        if (response.getServerID() == ModMeters[i].GetDeviceAddr() )
        {
            token &= 0x00ffffffL;   // mask out possible id
            ModMeters[i].handleMeterData(response, token);
            break;
        }
    }
}

void handleError(Error error, uint32_t token) 
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  debugD("Error response: %02X - %s", (int)me, (const char *)me);
 
}

/**
 * @brief prepare the ModBus communication
 * 
 * @param baudrate  : baudrate              (default: 9600)
 * @param iN        : number or meters      (1..4)
 * @param *dt       : type of Modbus meter  (array of device types
 * @param *devadr   : device adr            (array of device addresses
 */
void StartModBus(uint32_t baudrate, int iN, eMeterType *dt, uint16_t *devadr)
{
    debugD("StartModBus with %d devices at Baudrate %d", iN, baudrate);
    if (iN > MAX_METERS)
    {
        debugD("StartModBus clip to max. %d devices", MAX_METERS);
        iNMeters = MAX_METERS;
    }
    else
        iNMeters = iN;  
        
    for (int i = 0; i<iNMeters; i++)
    {
        debugD("%d: Type: %s, Addr: %d", i, ModMeters[i].MeterType2Text(*dt), *devadr);
        ModMeters[i].SetMeter(*dt, *devadr);
        ++dt;
        ++devadr;
    }

    // Set up Serial2 connected to Modbus RTU
    // ttgo lora pins for Serial2 => RX pin 35, TX pin 13, pin 17: RTS (Rx/Tx switch)
    pinMode(17, OUTPUT);
    // SDM support 8Bit, 1 stop, no parity
    Serial2.begin(baudrate, SERIAL_8N1, 35, 13);
    
    // Set up ModbusRTU client.
    // - provide onData handler function
    MB.onDataHandler(&handleData);
    // - provide onError handler function
    MB.onErrorHandler(&handleError);
    // Set message timeout to 2000ms
    MB.setTimeout(2000);
    // Start ModbusRTU background task
    MB.begin();

    // Start 'connect' request
    for (int i = 0; i<iNMeters; i++)
    {
        Error err = ModMeters[i].FireConnectRequest();
        if (err != SUCCESS) 
        {
            ModbusError e(err);
            debugD("Error creating request: %02X - %s", (int)e, (const char *)e);
        }
    }
} 

static long _tmMillis = 0;

/**
 * @brief Loop function for Modbus
 *        start cycle every 10s
 * 
 */
void ModBusHandle(void)
{
    if ((millis() - _tmMillis) > MODBUSCYCLE * 1000L)
    {
        debugD("inside ModBusHandle with %d", _tmMillis);
        for (int i = 0; i<iNMeters; i++)
            ModMeters[i].FireDataRequest();
        
        _tmMillis = millis();
    }
} 

ModBusMeter *GetMeterDataPtr(int idx)
{
    if ( (idx >= 0) && (idx < MAX_METERS))
        return & ModMeters[idx];
    else
        return NULL;
}

int GetNumberOfMeters(void)
{
    return iNMeters;
}
