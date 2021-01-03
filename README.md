# esp32ModbusGateway
 ModBus Gateway with Wifi and LORA Support

# Use case

Intention of this project is to connect "old" ModBus power meters to "usable" modern interfaces.
- Rest API / JSON
- Lora Client (The Things Network)

## JSON API description

The ModBusGateway uses a json API to communicate with a backend.

### Actions

  - `/api/meter` power meter data (`GET`)
    ```
    {
        "connected":true,
        "voltage":75.65258,
        "current":0,
        "power":0,
        "reactive_power":0,
        "frequency":49.89715,
        "energyout":0.02,
        "energyin":0,
        "u_phase_1":226.9928,
        "i_phase_1":0,
        "u_phase_2":0,
        "i_phase_2":0,
        "u_phase_3":0,
        "i_phase_3":0,
        "cycles":186,
        "ErrCnt":0
    }
    ```

  - `/api/status` system health (`GET`)
  - `/api/wlan` set WiFi configuration (`GET`)
  - `/api/restart` restart (`POST`)
  - `/api/settings` save settings (restarts) (`POST`)


## TTN Payload format

- ***Plain*** uses big endian format and generates json fields, e.g. useful for TTN console

- ***Packed*** uses little endian format and generates json fields

Hereafter described is the default *plain* format, which uses MSB bit numbering. Under /TTN in this repository you find some ready-to-go decoders which you may copy to your TTN console:

[**plain_decoder.js**](src/TTN/plain_decoder.js) | 
[**plain_converter.js**](src/TTN/plain_converter.js) |

**Port #1:** basic PowerMeter data

	byte 1-4:	(float): Energy In      [kWh]
    byte 5-8:   (float): Energy Out     [kWh]
    byte 9-12:  (float): current Power  [kW]

**Port #2:** Device status query result

  	byte 1-2:	Battery or USB Voltage [mV], 0 if no battery probe
	byte 3-10:	Uptime [seconds]
	byte 11: 	CPU temperature [°C]
	bytes 12-15:	Free RAM [bytes]
	bytes 16-17:	Last CPU reset reason [core 0, core 1]

**Port #7:** Environmental sensor data (only if device has feature BME)

	bytes 1-2:	Temperature [°C]
	bytes 3-4:	Pressure [hPa]
	bytes 5-6:	Humidity [%]
	bytes 7-8:	Indoor air quality index (0..500), see below

	Indoor air quality classification:
	0-50		good
	51-100		average
	101-150 	little bad
	151-200 	bad
	201-300 	worse
	301-500 	very bad

**Port #9:** Time/Date

  	bytes 1-4:	board's local time/date in UNIX epoch (number of seconds that have elapsed since January 1, 1970 (midnight UTC/GMT), not counting leap seconds) 


# Remote control

The device listenes for remote control commands on LoRaWAN Port 2. Multiple commands per downlink are possible by concatenating them.

Note: all settings are stored in NVRAM and will be reloaded when device starts.
(TODO)
	

# Prototype:

Test with TTGO Lora and SDM 610
<img src="Img/ModBusGatewayProto2.jpg">

Modbus Reply:
<img src="Img/SMD630ModBus.jpg">


# Hardware

**Supported ESP32 based boards**:

*LoRa & SPI*:
- TTGO: T1*, T2*, T3*, T-Beam, T-Fox



