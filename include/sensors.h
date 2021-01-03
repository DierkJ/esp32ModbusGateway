/**
**********************************************************************************************************************************************************************************************************************************
* @file:	sensors.h
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
#ifndef _SENSORS_H_INCLUDED
#define _SENSORS_H_INCLUDED

//
// environemet data
//

typedef struct {
    float temperature;  // in °C
    float pressure;     // in bar
    float altitude;     // in m
} sensor_data_t;


extern sensor_data_t g_SensorData;

extern void StartSensors(void);
extern void SensorsHandle(void);


#endif

