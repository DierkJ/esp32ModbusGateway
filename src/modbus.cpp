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
static const char TAG[] = __FILE__;

#include <SDM.h>                                                                //https://github.com/reaper7/SDM_Energy_Meter
#include "modbus.h"

#if !defined ( USE_HARDWARESERIAL )
  #error "This example works with Hardware Serial on esp32, please uncomment #define USE_HARDWARESERIAL in SDM_Config_User.h"
#endif


//esp32: pins for Serial2 => RX pin 35, TX pin 13
SDM sdm(Serial2, 9600, NOT_A_PIN, SERIAL_8N1, 35, 13);                            

/**
 * @brief
 * meter data storage
 */
modbus_meter_t g_modBusMeterData;
static modbus_config_t modBusConfig;

void ModBusTask(void *params);

//
// Error decoder
//
static const char *ModBusError2Text(uint16_t err)
{
    switch (err)
    {
        case SDM_ERR_NO_ERROR:  
            return "Success";
            break;
        case SDM_ERR_CRC_ERROR:
            return "CRC Error";
            break;
        case SDM_ERR_WRONG_BYTES:
            return "Wrong Bytes";
            break;
        case SDM_ERR_NOT_ENOUGHT_BYTES:
            return "Not enough bytes";
            break;
        case SDM_ERR_TIMEOUT:
            return "Timeout";
            break;
        default:
            return "unknown";
            break;
    }          
}

//
// device decoder
//
static const char *Device2Text(modbus_meter_type_t mt)
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
  
static float ReadNonNAN (uint16_t reg, uint8_t node)
{
    float fTmp = sdm.readVal(reg, node);
    if (isnan(fTmp))
        return 0.0;
    else
        return fTmp;
}


void StartModBus(modbus_meter_type_t dt, uint16_t devadr, uint32_t baudrate)
{
    // Setup the server as a separate task.
    ESP_LOGI(TAG, "Creating Modbus task... ");
    
    modBusConfig.eDeviceType = dt;
    modBusConfig.iDeviceAddr = devadr;
    modBusConfig.iBaudrate = baudrate;  // not yet configurable, because sdm instance is already created.
    memset( & g_modBusMeterData, 0, sizeof(g_modBusMeterData));

    sdm.begin();
    
    xTaskCreatePinnedToCore(ModBusTask, "modbus", 6144, NULL, 1, NULL, 1);
} 


void ModBusTask(void *params)
{
    ESP_LOGI(TAG, "inside ModBusTask with device: % at address: %d", Device2Text(modBusConfig.eDeviceType), modBusConfig.iDeviceAddr);
    while (1)
    {
        if (!g_modBusMeterData.fConnected)
        {
            //
            // read first register as connection test. Should work for all devices
            //
            float fTmp = sdm.readVal(SDM_PHASE_1_VOLTAGE, modBusConfig.iDeviceAddr);
            if ( !(isnan(fTmp)) && (sdm.getErrCode() == SDM_ERR_NO_ERROR))
            {
                ESP_LOGI(TAG, "Modbus connected");
                sdm.clearErrCount();
                g_modBusMeterData.fConnected = true;
            }
            else
            {
                ESP_LOGI(TAG, "Modbus failure (%d) %s", sdm.getErrCode(), ModBusError2Text(sdm.getErrCode()));
                // try again after 1 s
                delay(1000);
            }
        }
        else
        {
            if (modBusConfig.eDeviceType == MT_FINDER)
            {
                ESP_LOGI(TAG, "Finder not yet supported");
            }
            else if (modBusConfig.eDeviceType == MT_DDM)
            {
                ESP_LOGI(TAG, "DDM not yet supported");
            }
            else
            {
                g_modBusMeterData.fVoltage = ReadNonNAN(SDM_AVERAGE_L_TO_N_VOLTS, modBusConfig.iDeviceAddr);
                g_modBusMeterData.fCurrent = ReadNonNAN(SDM_AVERAGE_LINE_CURRENT, modBusConfig.iDeviceAddr);
                g_modBusMeterData.fPower = ReadNonNAN(SDM_TOTAL_SYSTEM_POWER, modBusConfig.iDeviceAddr);
                g_modBusMeterData.fFrequency = ReadNonNAN(SDM_FREQUENCY, modBusConfig.iDeviceAddr);
                g_modBusMeterData.fEnergyOut = ReadNonNAN(SDM_IMPORT_ACTIVE_ENERGY, modBusConfig.iDeviceAddr);
                g_modBusMeterData.fEnergyIn = ReadNonNAN(SDM_EXPORT_ACTIVE_ENERGY, modBusConfig.iDeviceAddr);

                if (modBusConfig.eDeviceType == MT_SDM630)
                {
                    // only for 3 phase meters (SDM 630)
                    int i;
                    for (i=0; i<3; i++)
                    {
                        g_modBusMeterData.fPhaseVoltage[i] = ReadNonNAN(SDM_PHASE_1_VOLTAGE + 2*i, modBusConfig.iDeviceAddr);  
                        g_modBusMeterData.fPhaseCurrent[i] = ReadNonNAN(SDM_PHASE_1_CURRENT + 2*i, modBusConfig.iDeviceAddr);  
                    }
                }
            }
            g_modBusMeterData.iCycles++;
            g_modBusMeterData.iErrCnt = sdm.getErrCount();
            delay(500);
        }        
    }
} 

