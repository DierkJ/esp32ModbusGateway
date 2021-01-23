/**
**********************************************************************************************************************************************************************************************************************************
* @file:	modbus.cpp
*
* @brief:	read modbus task
*
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
static const char TAG[] = __FILE__;

#include "globals.h"

#include "ModbusClientRTU.h"
#include "modbus.h"
#include "ModbusRegister.h"
#include "logging.h"



/**
 * @brief
 * meter data storage
 */
modbus_meter_t g_modBusMeterData;
static modbus_config_t modBusConfig;

//pins for Serial2 => RX pin 35, TX pin 13, pin 17: RTS (Rx/Tx switch)
ModbusClientRTU MB(Serial2, 17); 

#define MODBUSCYCLE          (10)       // read meter data every x s

// some tokens..
#define TOK_START   0x4711
#define TOK_FINAL   0x10000000L


//
// device decoder
//
const char *MeterType2Text(modbus_meter_type_t mt)
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

modbus_meter_type_t Text2MeterType(const String&  sDevText)
{
    modbus_meter_type_t dt = MT_UNKNOWN;

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

static float toFloat(ModbusMessage response)
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

static uint16_t toInt16(ModbusMessage response)
{
    uint16_t uTmp;
  
    ((uint8_t*)&uTmp)[1]= response[3];
    ((uint8_t*)&uTmp)[0]= response[4];
    return uTmp;
}

static uint32_t toInt32(ModbusMessage response)
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
void handleData(ModbusMessage response, uint32_t token) 
{
    debugV("Response: serverID=%d, FC=%d, Token=%08X, length=%d:", response.getServerID(), response.getFunctionCode(), token, response.size());

#if (0)
    for (auto& byte : response) 
    {
        debugD("%02X ", byte);
    }
#endif

    if (token == TOK_START)
        g_modBusMeterData.fConnected = true;
    else
    {
        modbus_meter_type_t mt = (modbus_meter_type_t) ((token >> 16) - 1);
        if (mt == MT_FINDER)
        {
            switch (token & 0xffffL)
            {
                case FINDER_FIRMWARE_VERSION:
                    g_modBusMeterData.uFWVersion = toInt16(response);
                    debugI("Finder firmware: %d.%d", g_modBusMeterData.uFWVersion/10, g_modBusMeterData.uFWVersion%10);
                    break;
                case FINDER_PHASE_1_VOLTAGE:
                    g_modBusMeterData.fPhaseVoltage[0] = (float) toInt16(response);
                    break;
                case FINDER_PHASE_1_CURRENT:
                    g_modBusMeterData.fPhaseCurrent[0] = (float)toInt16(response) * 0.1;
                     break;
                case FINDER_PHASE_1_POWER:
                    g_modBusMeterData.fPhasePower[0] = (float)toInt16(response) * 0.01;
                    break;
                 case FINDER_PHASE_1_REACTIVE_POWER:
                    g_modBusMeterData.fReactivePower[0] = (float)toInt16(response) * 0.01;
                    break;
                 case FINDER_IMPORT_ACTIVE_ENERGY:
                    g_modBusMeterData.fEnergyIn = (float)toInt32(response) * 0.01;
                    break;
                 case FINDER_EXPORT_ACTIVE_ENERGY:
                    g_modBusMeterData.fEnergyOut = (float)toInt32(response) * 0.01;
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
#if (0)                
                case SDM_AVERAGE_L_TO_N_VOLTS: 
                    g_modBusMeterData.fVoltage = toFloat( response);
                    break;
                case SDM_AVERAGE_LINE_CURRENT: 
                    g_modBusMeterData.fCurrent = toFloat( response);
                    break;
                case SDM_TOTAL_SYSTEM_POWER: 
                    g_modBusMeterData.fPower = toFloat( response);
                    break;
                case SDM_TOTAL_SYSTEM_REACTIVE_POWER: 
                    g_modBusMeterData.fReactivePower = toFloat( response);
                    break;
#endif 
                case SDM_FREQUENCY: 
                    g_modBusMeterData.fFrequency = toFloat( response);
                    break;
                case SDM_IMPORT_ACTIVE_ENERGY: 
                    g_modBusMeterData.fEnergyIn = toFloat( response);
                    break;
                case SDM_EXPORT_ACTIVE_ENERGY: 
                    g_modBusMeterData.fEnergyOut = toFloat( response);
                    break;
                case SDM_PHASE_1_VOLTAGE:
                    g_modBusMeterData.fPhaseVoltage[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_VOLTAGE:
                    g_modBusMeterData.fPhaseVoltage[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_VOLTAGE:
                    g_modBusMeterData.fPhaseVoltage[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_CURRENT:
                    g_modBusMeterData.fPhaseCurrent[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_CURRENT:
                    g_modBusMeterData.fPhaseCurrent[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_CURRENT:
                    g_modBusMeterData.fPhaseCurrent[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_POWER:
                    g_modBusMeterData.fPhasePower[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_POWER:
                    g_modBusMeterData.fPhasePower[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_POWER:
                    g_modBusMeterData.fPhasePower[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_APPARENT_POWER:
                    g_modBusMeterData.fApparentPower[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_APPARENT_POWER:
                    g_modBusMeterData.fApparentPower[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_APPARENT_POWER:
                    g_modBusMeterData.fApparentPower[2] = toFloat( response);
                    break;
                case SDM_PHASE_1_REACTIVE_POWER:
                    g_modBusMeterData.fReactivePower[0] = toFloat( response);
                    break;
                case SDM_PHASE_2_REACTIVE_POWER:
                    g_modBusMeterData.fReactivePower[1] = toFloat( response);
                    break;
                case SDM_PHASE_3_REACTIVE_POWER:
                    g_modBusMeterData.fReactivePower[2] = toFloat( response);
                    break;
            }
        }
        if (token & TOK_FINAL)
        {
            g_modBusMeterData.iCycles++;
        }
    }
}
/**
 * @brief Define an onError handler function to receive error responses
 * 
 * @param error error code
 * @param token user-supplied token to identify the causing request
 */
void handleError(Error error, uint32_t token) 
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  debugD("Error response: %02X - %s", (int)me, (const char *)me);
  g_modBusMeterData.iErrCnt++;
  g_modBusMeterData.iLastErr = error;
  
  // reset connected status
  g_modBusMeterData.fConnected = false;
  
}

Error FireConnectRequest(void)
{
    uint32_t uStartToken = TOK_START;

    if (modBusConfig.eDeviceType == MT_FINDER)
    {   
        // start with Firmware holding register
        return  MB.addRequest(uStartToken, modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_FIRMWARE_VERSION, 1);
//        return  MB.addRequest(uStartToken, modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_IMPORT_ACTIVE_ENERGY, 2);
    }
    else if (modBusConfig.eDeviceType == MT_DDM)
        return  MB.addRequest(uStartToken, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, DDM_PHASE_1_VOLTAGE, 2);
    else
        return  MB.addRequest(uStartToken, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_VOLTAGE, 2);

}

/**
 * @brief prepare the ModBus communication
 * 
 * @param dt        : type of Modbus meter  (default: MT_SDM630)
 * @param devadr    : device adr            (default: 1)
 * @param baudrate  : baudrate              (default: 9600)
 */
void StartModBus(modbus_meter_type_t dt, uint16_t devadr, uint32_t baudrate)
{

    modBusConfig.eDeviceType = dt;
    modBusConfig.iDeviceAddr = devadr;
    modBusConfig.iBaudrate = baudrate;  // not yet configurable, because sdm instance is already created.
    memset( & g_modBusMeterData, 0, sizeof(g_modBusMeterData));

    debugD("StartModBus with device: %s at address: %d", MeterType2Text(modBusConfig.eDeviceType), modBusConfig.iDeviceAddr);

    // Set up Serial2 connected to Modbus RTU
    // ttgo lora pins for Serial2 => RX pin 35, TX pin 13, pin 17: RTS (Rx/Tx switch)
    pinMode(17, OUTPUT);
    if (modBusConfig.eDeviceType == MT_FINDER)
    {
        // Finder supports 8bit, 2 stop, no parity, 
        Serial2.begin(modBusConfig.iBaudrate, SERIAL_8N2, 35, 13);
    }
    else
    {
        // SDM support 8Bit, 1 stop, no parity
        Serial2.begin(modBusConfig.iBaudrate, SERIAL_8N1, 35, 13);
    }
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
    Error err = FireConnectRequest();
    if (err != SUCCESS) 
    {
        ModbusError e(err);
        debugD("Error creating request: %02X - %s", (int)e, (const char *)e);
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

        if (g_modBusMeterData.fConnected)
        {
            //
            // put all requests in queue
            // code device type and register into token
            //
            uint32_t uT = ((uint32_t)modBusConfig.eDeviceType + 1) << 16;

            if (modBusConfig.eDeviceType == MT_FINDER)
            {
                MB.addRequest(uT + FINDER_PHASE_1_VOLTAGE , modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_VOLTAGE, 1);
                MB.addRequest(uT + FINDER_PHASE_1_CURRENT , modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_CURRENT, 1);
                MB.addRequest(uT + FINDER_PHASE_1_POWER , modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_POWER, 1);
                MB.addRequest(uT + FINDER_PHASE_1_REACTIVE_POWER , modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_PHASE_1_REACTIVE_POWER, 1);
                MB.addRequest(uT + FINDER_IMPORT_ACTIVE_ENERGY , modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_IMPORT_ACTIVE_ENERGY, 2);
                // mark last request in cycle:
                uT |= TOK_FINAL;                
                MB.addRequest(uT + FINDER_EXPORT_ACTIVE_ENERGY , modBusConfig.iDeviceAddr, READ_HOLD_REGISTER, FINDER_EXPORT_ACTIVE_ENERGY, 2);
            }
            else if (modBusConfig.eDeviceType == MT_DDM)
            {
                debugD("DDM not yet supported");
            }
            else
            {
                int i;
                for (i=0; i<3; i++)
                {
                    // 3 phase phase meters (SDM 630) scan all or only phase 1
                    if ( (modBusConfig.eDeviceType == MT_SDM630) || (i<1) )
                    {
                        MB.addRequest(uT + SDM_PHASE_1_VOLTAGE + 2*i, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_VOLTAGE + 2*i, 2);
                        MB.addRequest(uT + SDM_PHASE_1_CURRENT + 2*i, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_CURRENT + 2*i, 2);
                        MB.addRequest(uT + SDM_PHASE_1_POWER + 2*i, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_POWER + 2*i, 2);
                        MB.addRequest(uT + SDM_PHASE_1_APPARENT_POWER + 2*i, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_APPARENT_POWER + 2*i, 2);
                        MB.addRequest(uT + SDM_PHASE_1_REACTIVE_POWER + 2*i, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_PHASE_1_REACTIVE_POWER + 2*i, 2);
                    }
                }
#if (0)
                if (modBusConfig.eDeviceType == MT_SDM630)
                {
                    MB.addRequest(uT + SDM_AVERAGE_L_TO_N_VOLTS , modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_AVERAGE_L_TO_N_VOLTS, 2);
                    MB.addRequest(uT + SDM_AVERAGE_LINE_CURRENT, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_AVERAGE_LINE_CURRENT, 2);
                    MB.addRequest(uT + SDM_TOTAL_SYSTEM_POWER, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_TOTAL_SYSTEM_POWER, 2);
                    MB.addRequest(uT + SDM_TOTAL_SYSTEM_REACTIVE_POWER, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_TOTAL_SYSTEM_REACTIVE_POWER, 2);
                }
#endif
                MB.addRequest(uT + SDM_FREQUENCY, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_FREQUENCY, 2);
                MB.addRequest(uT + SDM_IMPORT_ACTIVE_ENERGY, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_IMPORT_ACTIVE_ENERGY, 2);
                // mark last request in cycle:
                uT |= TOK_FINAL;
                MB.addRequest(uT + SDM_EXPORT_ACTIVE_ENERGY, modBusConfig.iDeviceAddr, READ_INPUT_REGISTER, SDM_EXPORT_ACTIVE_ENERGY, 2);
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
        _tmMillis = millis();
    }
} 

