# AIO-AIR-V2

A compact ESP32 based air-quality monitor combining dedicated sensors for CO₂, particulate matter, VOCs, NOx, temperature and humidity.
![Alt text](https://github.com/CXD8/AIO-AIR-v2/blob/71408ca1630c75612f7a3e87ab16bef816e1d59f/pcb/images/AIO-AIR-v2-pcb-front.PNG)

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


## BOM
| Item | References | Value | Footprint | Quantity | Link |
| ---: | :--- | :--- | :--- | :---: | :--- |
| 1 | C1, C4, C6, C9, C11, C13, C14, C16, C19 | CL10B104KB8NNNC | CL10B104KB8NNNC | 9 | [Link](https://www.lcsc.com/product-detail/C1591.html) |
| 2 | C2, C10, C17, C18 | CL10A106KP8NNNC | CL10A106KP8NNNC | 4 | [Link](https://www.lcsc.com/product-detail/C19702.html) |
| 3 | C5, C7, C12, C15 | CL10A226MP8NUNE | CL10A226MP8NUNE | 4 | [Link](https://www.lcsc.com/product-detail/C86295.html) |
| 4 | R1, R4 | RC0603FR-0710KL | RC0603FR-0710KL | 2 | [Link](https://www.lcsc.com/product-detail/C98220.html) |
| 5 | R2, R3 | RC0603FR-075K1L | RC0603FR-075K1L | 2 | [Link](https://www.lcsc.com/product-detail/C105580.html) |
| 6 | U4, U7 | ~ | power_1 | 2 | — |
| 7 | U1 | ESP32-WROOM-32U_8MB | ESP32-WROOM-32U_8MB | 1 | [Link](https://www.lcsc.com/product-detail/C328062.html) |
| 8 | U2 | AP2114H-3_3TRG1 | AP2114H-3_3TRG1 | 1 | [Link](https://www.lcsc.com/product-detail/C150716.html) |
| 9 | U3 | UART DOWNLOAD | 2_54-1_4 | 1 | — |
| 10 | U5 | DISPLAY_1 | 2_54-1_4 | 1 | [Link](https://www.aliexpress.com/item/1005012793358185.html) |
| 11 | U6 | DISPLAY_2 | 2_54-1_4 | 1 | [Link](https://www.aliexpress.com/item/1005012793358185.html) |
| 12 | SW1 | TS-1088-AR02016 | TS-1088-AR02016 | 1 | [Link](https://www.lcsc.com/product-detail/C720477.html) |
| 13 | CN1 | SGP41 | PH2_0-4P | 1 | [Link](https://sensirion.com/products/catalog/SGP41) |
| 14 | CN2 | SCD30 | B5B-PH-K-S-_LF_SN | 1 | [Link](https://sensirion.com/products/catalog/SCD30) |
| 15 | CN3 | SPS30 | B5B-PH-K-S-_LF_SN | 1 | [Link](https://sensirion.com/products/catalog/SPS30) |
| 16 | CN4 | LUX SENSOR | B5B-PH-K-S-_LF_SN | 1 | [Link](https://www.aliexpress.com/item/1005007790877449.html) |
| 17 | USB1 | TYPE-C_16P_QTWT | TYPE-C_16P_QTWT | 1 | [Link](https://www.lcsc.com/product-detail/C5187472.html) |
