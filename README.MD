# PilotSpa - Balboa Spa Control System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESP32](https://img.shields.io/badge/ESP32-Ready-blue.svg)](https://www.espressif.com/)
[![ESPHome Compatible](https://img.shields.io/badge/ESPHome-Compatible-green.svg)](https://esphome.io/)
[![Home Assistant](https://img.shields.io/badge/Home%20Assistant-Compatible-blue.svg)](https://www.home-assistant.io/)

## Overview

PilotSpa is a complete ESP32-based control system for Balboa spa systems featuring the VL801D DeluxeSerie panel. It provides two implementation approaches:
- **Arduino IDE** with integrated web UI and MQTT
- **ESPHome** with native Home Assistant integration

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/carte-electronique-sf273-spaform-aquamarine.jpg" width="400" alt="PCB Board"> <img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/VL801D-balboa-panel-spa-studio-WEB_ex.jpg" width="400" alt="Balboa Panel">

---

## Table of Contents

- [Hardware Specifications](#hardware-specifications)
  - [RJ45 Pins](#rj45-pins)
  - [Pin Configuration](#pin-configuration)
- [Protocol Analysis](#protocol-analysis)
  - [Display Data](#display-data)
  - [Button Data](#button-data)
- [Upload Methods](#upload-methods)
  - [Arduino IDE](#option-1-via-arduino-ide)
  - [ESPHome](#option-2-via-esphome)
- [Features](#features)
- [Installation](#installation)
- [PCB Fabrication](#pcb-fabrication)
- [References](#references)

---

## Hardware Specifications

### RJ45 Pins

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/RJ45.png" width="200" alt="RJ45 Pinout">

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

### Pin Configuration

**Critical Signal Lines:**
- **PIN 6 (Yellow)**: Clock signal
- **PIN 5 (Blue)**: Display data
- **PIN 3 (Pink)**: Button data

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-1.jpg" width="800" alt="Signal Analysis 1"> 
<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-2.jpg" width="800" alt="Signal Analysis 2">

---

## Protocol Analysis

### Signal Characteristics

- **Clock Pulses**: 7 pulses per display data chunk (71 bits), 4 pulses for button data
- **Complete Cycle**: 75 pulses total
- **Data Chunks**: 6 display data chunks per cycle
- **Cycle Frequency**: Every 10 ms (100 Hz)
- **Complete Cycle Duration**: 3.54 ms
- **Display Data Bit Duration**: 23.75 μs
- **Button Data Bit Duration**: 71.5 μs

### Display Data

**Example: Temperature Display 29.5°C**

The display encodes values using BCD to 7-segment LCD format (7 bits, LSB always 0):

```
2 = 1101101(0) → Digit 1
9 = 1110011(0) → Digit 2
5 = 1011011(0) → Digit 3
C = 1001110(0) → Digit 4
```

**Bit Mapping Table:**

| Chunk 1 | Chunk 2 | Chunk 3 | Chunk 4 | Chunk 5 (Status Bits) | 
| --- | --- | --- | --- | --- |
| LCD Seg 1 | LCD Seg 2 | LCD Seg 3 | LCD Seg 4 | Bit 29: Temp ±  |   
| | | | | Bit 39: Button  | 
| | | | | Bit 40: Button  | 
| | | | | Bit 42: Blower  | 
| | | | | Bit 44: Pump 3  |
| | | | | Bit 47: Light   |
| | | | | Bit 48: Pump 1  |
| | | | | Bit 49: Pump 2  |
| | | | | Bit 59: Standard|
| | | | | Bit 60: Eco     |

### Button Data

Control commands are encoded as 4-bit sequences:

| Button | Binary Code | 
| --- | --- |
| Mode | 1 0 0 0 |
| Temperature ↑ | 1 1 1 0 |
| Temperature ↓ | 1 1 1 1 |  
| Light | 1 0 1 1 |
| Pump 1 | 1 0 0 0 | 
| Pump 2 | 1 0 1 0 |
| Blower | 1 1 0 1 |

---

## ESP32 Connection Map

- **Clock Pin**: GPIO 23
- **Read Pin (Display Data)**: GPIO 22
- **Write Pin (Button Control)**: GPIO 13

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

---

## PCB Fabrication & Schematic

### PCB Design

<img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_3D_view.jpg" width="303" alt="PCB 3D View"> <img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_2D_view.jpg" width="400" alt="PCB 2D View">

### Schematic Diagram

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/PCB%20Layout/PCB_Schematic_Balboa.jpg" width="800" alt="PCB Schematic"> 

**Gerber Files**: [PCB Balboa-GS510SZ](https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/tree/main/extras/PCB%20layout)

---

## Features

### Core Capabilities

- **Wi-Fi Connectivity**
  - Automatic connection at startup
  - SoftAP fallback mode for configuration
  - Configurable SSID and password storage in EEPROM

- **Web Server Interface**
  - HTML-based configuration dashboard
  - Real-time spa status display
  - Responsive design with W3.CSS framework

- **MQTT Integration**
  - Publish spa telemetry data
  - Subscribe to control commands
  - Configurable broker endpoint

- **Spa Control**
  - Temperature adjustment (↑/↓)
  - Pump control (1, 2, 3)
  - Blower control
  - Light control
  - Mode switching (Standard/Eco)

- **Over-The-Air Updates**
  - ElegantOTA support (Arduino IDE)
  - Built-in OTA (ESPHome)
  - Wireless firmware updates

- **Home Assistant Integration**
  - MQTT discovery
  - Native ESPHome integration
  - Sensor entities for all controls

---

## Upload Methods

### Option 1: Via Arduino IDE

#### Requirements
- Arduino IDE 1.8.19 or higher
- ESP32 boards package (v2.0.0+)
- CH340/CH341 USB driver (if needed)

#### Setup Steps

1. **Install Arduino IDE** and configure ESP32 boards
   ```
   Tools → Board Manager → Search "esp32" → Install
   ```

2. **Add Library Dependencies**
   ```
   Sketch → Include Library → Manage Libraries
   Search and install: ArduinoJson, PubSubClient, WiFiManager
   ```

3. **Prepare Project Structure**
   - Copy `src/Balboa_GS_Interface.cpp` and `.h` to sketch folder
   - Create `data/` subdirectory with SPIFFS files:
     - `config.html` - Configuration interface
     - `index.html` - Main dashboard
     - `pilotage.html` - Control panel
     - `w3.css` - Stylesheet

4. **Upload SPIFFS Files**
   ```
   Tools → ESP32 Sketch Data Upload
   ```

5. **Compile and Upload**
   ```
   Select Board: ESP32 Dev Module
   Select Port: COM# (your ESP32 port)
   Upload Speed: 921600
   Click: Upload
   ```

#### Resources

| Item | Location |
|------|----------|
| Source Code | `src/` |
| UI Files | `examples/data/` |
| Complete Example | `examples/Balboa_GS_HomeAssistant.ino` |

---

### Option 2: Via ESPHome

#### Requirements
- Home Assistant instance with ESPHome add-on
- YAML configuration knowledge
- USB connection to ESP32

#### Setup Steps

1. **Create ESPHome Device**
   ```yaml
   esphome:
     name: balboa-spa
     platform: ESP32
     board: esp32devkitc
   ```

2. **Add Custom Component**
   - Copy `esphome/components/balboa_spa/` to your ESPHome config directory
   - Reference in your device YAML:
   ```yaml
   external_components:
     - source: local
       components: [balboa_spa]
   ```

3. **Load Configuration**
   - Use provided `esphome/balboa_spa.yaml` as template
   - Customize Wi-Fi and MQTT settings

4. **Build and Upload**
   ```
   ESPHome Dashboard → Select Device → Build & Upload
   ```

#### ESPHome Component Files

| File | Purpose |
|------|---------|
| `__init__.py` | Component initialization |
| `sensor.py` | Temperature and status sensors |
| `text_sensor.py` | Text-based information |
| `binary_sensor.py` | Boolean states |
| `climate.py` | Thermostat control |
| `balboa_spa.h` | C++ interface code |

#### ESPHome Advantages

✅ Native Home Assistant integration  
✅ No external web server needed  
✅ Simplified OTA updates  
✅ YAML-based configuration  
✅ Active community support  

---

## Comparison: Arduino IDE vs ESPHome

| Aspect | Arduino IDE | ESPHome |
|--------|-------------|---------|
| **Learning Curve** | Moderate | Low |
| **Web Interface** | Included | Via Home Assistant |
| **Configuration** | C++ Code | YAML |
| **Home Assistant** | MQTT Bridge | Native Integration |
| **OTA Updates** | Manual Setup (ElegantOTA) | Built-in |
| **Flexibility** | High (Full C++ code) | Medium (YAML-based) |
| **Maintenance** | Self-managed | Community-maintained |

---

## Installation & First Setup

### Initial Configuration

1. **Flash SPIFF to ESP32**
   ```
   Arduino IDE: Tools → ESP32 Sketch Data Upload
   ```

2. **Upload Firmware**
   ```
   Arduino IDE: Sketch → Upload
   ```

3. **Connect to WiFi Setup Portal**
   - Device broadcasts: `OpenSpa-config`
   - Password: `openspa2801`
   - Open browser: http://192.168.4.1

4. **Configure WiFi Settings**
   - Enter your local network credentials
   - Set MQTT broker details (if used)
   - Save configuration

5. **Connect to Local Network**
   - Access device on local IP (displayed during setup)
   - Example: http://192.168.1.33

---

## Troubleshooting

### Common Issues

**WiFi Connection Fails**
- Verify SSID/password in config page
- Check router is broadcasting SSID
- Restart device if necessary

**No Web Interface**
- Confirm SPIFF files uploaded
- Check IP address in serial monitor
- Try factory reset: Press RESET button 3x quickly

**MQTT Connection Issues**
- Verify broker is running and accessible
- Check credentials and port settings
- Monitor serial output for error messages

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

---

## References

### Technical Documentation

- [Balboa GS510SZ with VL801D MQTT Control](https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie--MQTT)
- [Balboa Display Analysis & Protocol](https://docs.google.com/document/u/0/d/1xuc5N6-Q7J7bQxBARYjUywhqkIFjOnh8vAHqEya5QAI/mobilebasic)
- [Balboa Display Measurement Reference](https://github.com/NickB1/OpenSpa/blob/master/documents/Balboa/Balboa_Display_Measurements.pdf)
- [Internet of Spas - Technical Overview](https://www.olivierhill.ca/archives/72-The-Internet-of-Spas.html)
- [7-Segment LED Display with Shift Register](https://create.arduino.cc/projecthub/meljr/7-segment-led-displays-102-using-a-shift-register-6b6976)

### Related Projects

- [GL2000 Series Controller](https://github.com/netmindz/balboa_GL_ML_spa_control)
- [Balboa Worldwide App Protocol Analysis](https://github.com/ccutrer/balboa_worldwide_app)
- [Balboa GS510SZ Full Implementation](https://github.com/MagnusPer/Balboa-GS510SZ/)

---

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request for any improvements, bug fixes, or documentation enhancements.

---

**Last Updated**: 2026-06-15  
**Version**: 1.0  
**Supported Hardware**: ESP32, Balboa VL801D DeluxeSerie Panel
