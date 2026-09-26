# AIO-AIR-V2
# Air Monitor

A compact ESP32 based air-quality monitor combining dedicated sensors for CO₂, particulate matter, VOCs, NOx, temperature and humidity.

## Features

- **True NDIR CO₂ measurement w/ temperature and humidity** Sensirion SCD30
- **Particulate monitoring** Sensirion SPS30
  - PM1.0
  - PM2.5
  - PM4.0
  - PM10
- **VOC & NOx monitoring** Sensirion SGP41
- **240×320 colour TFT display**
- **ESP32-S3 powered**
- **Wi-Fi connectivity**
- **Home Assistant integration**
  - Live sensor data
  - Individual sensor controls
  - Remote sensor enable/disable
  - Historical data
- **Real-time air-quality status indicators**
- **Compact all-in-one design**

## How to Use

### 1. Flash the Firmware

1. Download or clone this repository.
2. Open the project in **Arduino IDE**.
3. Install the required libraries.
4. Select your **ESP32-S3** board and the correct COM/USB port.
5. Connect the ESP32-S3 via USB.
6. Upload the firmware to the board.

### 2. Connect the Sensors

### 3. Configure Wi-Fi

Enter your Wi-Fi credentials in the firmware configuration

Flash the firmware after changing the credentials.

### 3. Home Assistant
Once connected to Wi-Fi, the device can be added to **Home Assistant**.

Home Assistant provides access to the sensor readings and allows supported sensor settings to be changed remotely, including:

- Sensor enable/disable
- Historical data
- + more
