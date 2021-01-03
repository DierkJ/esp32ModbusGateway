/**
**********************************************************************************************************************************************************************************************************************************
* @file:	sensors.cpp
*
* @brief:	Sensor functions
* @author:	Dierk Arp
* @date:	20210103 16:01:24 
* @version:	1.0
*
* @copyright:	(c) 2021 Team HAHIS
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

// Libaries for BMI388
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"


#include "globals.h"
#include "sensors.h"

#define SEALEVELPRESSURE_HPA (1013.25)


/**
 * @brief
 * sensor data storage
 */
sensor_data_t g_SensorData;

//////////////////////////////////////////////////////////////////////////////////
// BMP388 Instance
Adafruit_BMP3XX bmp; // I2C

/**
 * @brief prepare the sensors communication
 * 
 * 
 */
void StartSensors(void)
{

    // Set up oversampling and filter initialization
    I2C_MUTEX_LOCK();
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_2X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_32X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_127);
    bool bOk = bmp.performReading();
    I2C_MUTEX_UNLOCK();
    if (! bOk)
    {
        ESP_LOGI(TAG,"Failed to read from BMP388");
    }
    else
    {
        ESP_LOGI(TAG, "BMP ready");
    }
} 

static long _tmMillis = 0;

/**
 * @brief Loop function for sensors
 *        start cycle every 1s
 * 
 */
void SensorsHandle(void)
{
    //ESP_LOGI(TAG, "inside sensorsHandle with %d / %d", millis(), _tmMillis);
    
    if ((millis() - _tmMillis) > 1000L)
    {
        ESP_LOGI(TAG, "inside sensorsHandle with %d", _tmMillis);
        
        I2C_MUTEX_LOCK();
        if (! bmp.performReading()) 
        {
            ESP_LOGI(TAG,"Failed to read from BMP388");
        }
        else
        {
            g_SensorData.temperature = bmp.temperature;
            g_SensorData.pressure = bmp.pressure;
            g_SensorData.altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        }
        I2C_MUTEX_UNLOCK();
        ESP_LOGI(TAG, "temp: %f °C, pressure: %f hPa", g_SensorData.temperature, g_SensorData.pressure);
        _tmMillis = millis();
    }
} 

