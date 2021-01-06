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

// Libaries for BMI388
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"


#include "globals.h"
#include "sensors.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define SENSORCYCLE          (8)       // read env. data every x s


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

    bool bOk = bmp.begin_I2C();
    if (bOk) 
    {
        bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_2X);
        bmp.setPressureOversampling(BMP3_OVERSAMPLING_32X);
        bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_127);
        bOk = bmp.performReading();
    }
    I2C_MUTEX_UNLOCK();
    
    if (! bOk)
    {
        debugE("Failed to read from BMP388");
    }
    else
    {
        debugI( "BMP ready");
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
    //debugD( "inside sensorsHandle with %d / %d", millis(), _tmMillis);
    
    if ((millis() - _tmMillis) > SENSORCYCLE * 1000L)
    {
        debugD( "inside sensorsHandle with %d", _tmMillis);
        
        I2C_MUTEX_LOCK();
        if (! bmp.performReading()) 
        {
            debugE("Failed to read from BMP388");
        }
        else
        {
            g_SensorData.temperature = bmp.temperature;     // °C
            g_SensorData.pressure = bmp.pressure/100.0;     // hPa
            g_SensorData.altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
            debugD( "temp: %f C, pressure: %f hPa", g_SensorData.temperature, g_SensorData.pressure);
        }
        I2C_MUTEX_UNLOCK();
        _tmMillis = millis();
    }
} 

