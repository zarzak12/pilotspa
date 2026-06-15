# PilotSpa

## Balboa Spa Control with ESP32

Balboa with panel VL801D DeluxeSerie and board SF273

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/carte-electronique-sf273-spaform-aquamarine.jpg" width="400"> <img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/VL801D-balboa-panel-spa-studio-WEB_ex.jpg" width="400">
## RJ45 Pins
<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/RJ45.png" width="200">

| PIN           | Description             | 
| ------------- | ------------------------|
| PIN 1         | Functionality unclear   |
| PIN 2         | Functionality unclear   |
| PIN 3         | Button data             |  
| PIN 4         | GND                     |
| PIN 5         | Display data            | 
| PIN 6         | Clock                   |
| PIN 7         | 5 VDC                   |  
| PIN 8         | Floating output         |

##PIN 6 (Yellow - clock), PIN 5 (Blue - display data), PIN 3 (Pink - button data)

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-1.jpg" width="800"> 
<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-2.jpg" width="800"> 

#### Conclusion

- The clock is pulsing 7 times for each chunk of display data, totaling 71 bits and 4 times for button data. A complete cycle = 75 pulses. <br />
- Total of 6 display data chunks in each cycle <br />
- Each cycle is repeated every 10 ms <br />
- One complete cycle duration: 3.54 ms
- Bit duration for display data is 23.75 microseconds <br />
- Bit duration for button data is 71.5 microseconds <br />

#### Display Data

Display shows **29.5C** - How does that translate into the bitstream? The first 4 chunks are coded in BCD to 7 segment LCD (see reference) but only with 7 bits since the LSB is always (0). <br />
2 = 1101101(0) = (Chunk 1) <br />
9 = 1110011(0) = (Chunk 2) <br />
5 = 1011011(0) = (Chunk 3) <br />
C = 1001110(0) = (Chunk 4) <br />

| Chunk 4 - bit(0-7) | Chunk 3 - bit(7-14) | Chunk 2 - bit(14-21) | Chunk 4 - bit(21-28) | Chunk 5 - bit(29-71) | 
| ------------------ | ------------------- | ------------------   | -------------------- | -------------------- |                     
| LCD segment 1      |  LCD segment 2      |   LCD Segment 3      |   LCD segment 4      | 29 - Button down/up  |   
|                    |                     |                      |                      |       ???            |  
|                    |                     |                      |                      | 39 - Button          | 
|                    |                     |                      |                      | 40 - Button          | 
|                    |                     |                      |                      | 42 - Blower          | 
|                    |                     |                      |                      | 44 - Pump3           |
|                    |                     |                      |                      | 47 - Light           |
|                    |                     |                      |                      | 48 - Pump1           |
|                    |                     |                      |                      | 49 - Pump2           |
|                    |                     |                      |                      | 59 - Standard mode   | 
|                    |                     |                      |                      | 60 - Eco mode        | 
|                    |                     |                      |                      |        ???           |  


<img src="https://github.com/MagnusPer/Balboa-GS510SZ/blob/main/extras/docs/Balboa_GS_Interface.jpg" width="1000">

For the connection I chose an ESP 32 and connected:

Clock pin: 23<br/>
Read pin: 22<br/>
Write Pin: 13<br/>

#### Button Data

|  Button   |    Decoding bit(0-3) | 
| --------- | -------------------- |
| Mode      |   1 0 0 0            |
| Temp up   |   1 1 1 0            |
| Temp down |   1 1 1 1            |  
| Light     |   1 0 1 1            |
| Pump 1    |   1 0 0 0            | 
| Pump 2    |   1 0 1 0            |
| Blower    |   1 1 0 1            |

## PCB Fabrication

<img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_3D_view.jpg" width="303"> <img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_2D_view.jpg" width="400">

## Schema
<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/PCB%20Layout/PCB_Schematic_Balboa.jpg" width="800"> 

Gerber files located here: [PCB Balboa-GS510SZ](https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/tree/main/extras/PCB%20layout )

#### Overview

This project is an application to control a Balboa spa system using an ESP32, featuring a web user interface and MQTT integration. Here's a summary of the main features:

- **Wi-Fi Connection**: The code handles Wi-Fi connection at startup. If Wi-Fi connection fails, the code switches to SoftAP mode to allow for configuration. A web server is configured for Wi-Fi configuration management.

- **EEPROM**: Configuration information such as SSID, password, etc., is stored in EEPROM.

- **Web Server**: A web server is configured with HTML pages to display the spa's status and allow configuration.

## ESP32 Upload Methods

### Option 1: Via Arduino IDE

#### Steps:
1. **Install Arduino IDE** and add ESP32 boards via the board manager
2. **Copy the source code** from the `src/` folder (Balboa_GS_Interface.cpp and .h)
3. **Upload UI files** from the `examples/data/` folder to the ESP32 SPIFFS flash memory:
   - `config.html` - Wi-Fi configuration page
   - `index.html` - Main control interface
   - `pilotage.html` - Control interface
   - `w3.css` - Stylesheet
4. **Compile and upload** to the ESP32

#### Resources:
- Source folder: `src/`
- UI files: `examples/data/`
- Complete example: `examples/Balboa_GS_HomeAssistant.ino`

---

### Option 2: Via ESPHome

#### Steps:
1. **Configure ESPHome** with the YAML configuration file
2. **Use the custom component** located in `esphome/components/balboa_spa/`
3. **Load the configuration** `esphome/balboa_spa.yaml` into your ESPHome instance
4. **Compile and upload** via the ESPHome interface

#### ESPHome Advantages:
- Direct integration with Home Assistant
- Simplified dependency management
- No need for external Web UI (accessible via Home Assistant)
- Built-in OTA updates

#### ESPHome Component:
- Main file: `esphome/components/balboa_spa/__init__.py`
- Sensors: `sensor.py`, `text_sensor.py`, `binary_sensor.py`
- Climate control: `climate.py`
- C++ headers: `balboa_spa.h`

---

## Comparison of Both Approaches

| Feature | Arduino IDE | ESPHome |
|---------|-------------|---------|
| Complexity | Medium | Low |
| Web UI | Yes (included) | No (via Home Assistant) |
| HA Integration | MQTT | Native |
| OTA Update | Manual (ElegantOTA) | Built-in |
| Configuration | C++ Code | Simple YAML |

---

## Overview

This project is an application to control a Balboa spa system using an ESP32, featuring a web user interface and MQTT integration. Here's a summary of the main features:

- **Wi-Fi Connection**: The code handles Wi-Fi connection at startup. If Wi-Fi connection fails, the code switches to SoftAP mode to allow for configuration. A web server is configured for Wi-Fi configuration management.

- **EEPROM**: Configuration information such as SSID, password, etc., is stored in EEPROM.

- **Web Server**: A web server is configured with HTML pages to display the spa's status and allow configuration.

- **MQTT**: The code connects to an MQTT broker with configuration information. Spa status data is published via MQTT.

- **OTA (Over-The-Air)**: The code supports Over-The-Air updates, allowing wireless firmware updates.

- **Balboa Interface**: The code uses a Balboa interface to interact with the spa system. Commands are sent to the Balboa system based on user actions.

- **Home Assistant Integration**: Spa data is exposed as sensors, switches, etc., for integration with Home Assistant. Virtual buttons on the user interface trigger commands on the Balboa system.

- **Timestamping**: The ESP32 is configured to use NTP time for timestamping.

- **Debugging**: Debug information is displayed on the serial port. A specific configuration page is protected with a username and password.

- **Timing Mechanism (Ticker)**: A timing mechanism (Ticker) is used to periodically check the Wi-Fi connection.

- **Wi-Fi Reconnection Function**: In case of Wi-Fi connection loss, the code attempts to reconnect.

---

## First Installation

- Upload SPIFF to ESP32
- Upload compiled code to ESP32
- Connect your Wi-Fi devices to the OpenSpa-config Wi-Fi with password openspa2801
- Call the IP address of OpenSpa-config (ex: http://192.168.4.1)
- Set all information on the settings page
- Call the new IP address connected to your local network (ex: http://192.168.1.33)

## Helpful References

- https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie--MQTT
- https://docs.google.com/document/u/0/d/1xuc5N6-Q7J7bQxBARYjUywhqkIFjOnh8vAHqEya5QAI/mobilebasic
- https://github.com/NickB1/OpenSpa/blob/master/documents/Balboa/Balboa_Display_Measurements.pdf
- https://www.olivierhill.ca/archives/72-The-Internet-of-Spas.html
- https://create.arduino.cc/projecthub/meljr/7-segment-led-displays-102-using-a-shift-register-6b6976

## Other Balboa Projects

- GL2000 Series https://github.com/netmindz/balboa_GL_ML_spa_control
- BP Series https://github.com/ccutrer/balboa_worldwide_app
- https://github.com/MagnusPer/Balboa-GS510SZ/
