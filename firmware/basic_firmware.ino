#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_SCD30.h>

// ==========================================
// PIN CONFIGURATION
// ==========================================
constexpr int PIN_I2C_SDA = 8;
constexpr int PIN_I2C_SCL = 3;

constexpr int PIN_SPS30_RX = 18; // ESP32 RX from SPS30 TX
constexpr int PIN_SPS30_TX = 17; // ESP32 TX to SPS30 RX

constexpr int PIN_TFT_CS   = 10;
constexpr int PIN_TFT_DC   = 4;
constexpr int PIN_TFT_MOSI = 11;
constexpr int PIN_TFT_SCK  = 12;
constexpr int PIN_TFT_MISO = 13;
constexpr int PIN_TFT_RST  = -1; // -1 if tied to EN/3.3V
constexpr int PIN_TFT_BL   = 5;  // Backlight GPIO (set -1 if disconnected)

// ==========================================
// PERIPHERAL INSTANCES
// ==========================================
Adafruit_ST7789 tft = Adafruit_ST7789(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_MOSI, PIN_TFT_SCK, PIN_TFT_RST);
Adafruit_SCD30 scd30;
HardwareSerial spsSerial(1);

// Sensor statuses
bool scd30_ok = false;
bool sgp41_ok = false;
bool sps30_ok = false;

// ==========================================
// SGP41 MINIMAL DRIVER (I2C Address 0x59)
// ==========================================
constexpr uint8_t SGP41_ADDR = 0x59;

uint8_t sensirionCrc8(const uint8_t* data, uint8_t len) {
  uint8_t crc = 0xFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) crc = (crc << 1) ^ 0x31;
      else crc = (crc << 1);
    }
  }
  return crc;
}

bool readSGP41(uint16_t &rawVoc, uint16_t &rawNox, float tempC = 25.0f, float rhPercent = 50.0f) {
  // Convert T and RH to SGP41 ticks
  uint16_t rhTicks = (uint16_t)((rhPercent * 65535.0f) / 100.0f);
  uint16_t tTicks  = (uint16_t)(((tempC + 45.0f) * 65535.0f) / 175.0f);

  uint8_t cmd[8];
  cmd[0] = 0x26; // measure_raw_signals command (0x2612)
  cmd[1] = 0x12;
  cmd[2] = rhTicks >> 8;
  cmd[3] = rhTicks & 0xFF;
  cmd[4] = sensirionCrc8(&cmd[2], 2);
  cmd[5] = tTicks >> 8;
  cmd[6] = tTicks & 0xFF;
  cmd[7] = sensirionCrc8(&cmd[5], 2);

  Wire.beginTransmission(SGP41_ADDR);
  Wire.write(cmd, sizeof(cmd));
  if (Wire.endTransmission() != 0) return false;

  delay(50); // SGP41 execution time: ~50ms

  if (Wire.requestFrom(SGP41_ADDR, (uint8_t)6) != 6) return false;

  uint8_t rx[6];
  for (int i = 0; i < 6; i++) rx[i] = Wire.read();

  if (sensirionCrc8(&rx[0], 2) != rx[2]) return false;
  if (sensirionCrc8(&rx[3], 2) != rx[5]) return false;

  rawVoc = (uint16_t(rx[0]) << 8) | rx[1];
  rawNox = (uint16_t(rx[3]) << 8) | rx[4];
  return true;
}

// ==========================================
// SPS30 MINIMAL UART SHDLC DRIVER
// ==========================================
void sps30SendCmd(uint8_t cmd, const uint8_t* data = nullptr, uint8_t len = 0) {
  uint8_t buf[32];
  uint8_t idx = 0;
  buf[idx++] = 0x7E; // Start
  buf[idx++] = 0x00; // Address
  buf[idx++] = cmd;
  buf[idx++] = len;

  uint8_t checksum = 0x00 + cmd + len;
  for (uint8_t i = 0; i < len; i++) {
    buf[idx++] = data[i];
    checksum += data[i];
  }
  buf[idx++] = ~checksum; // Inverted sum checksum
  buf[idx++] = 0x7E; // Stop

  spsSerial.write(buf, idx);
}

void sps30StartMeasurement() {
  uint8_t payload[2] = { 0x01, 0x03 }; // 0x01: Start, 0x03: IEEE754 float mode
  sps30SendCmd(0x00, payload, 2);
}

bool sps30ReadValues(float &pm1, float &pm25, float &pm10) {
  // Clear any stray input
  while (spsSerial.available()) spsSerial.read();

  sps30SendCmd(0x03); // Command 0x03: Read Measured Values
  
  uint32_t start = millis();
  uint8_t rx[80];
  uint8_t count = 0;
  bool inFrame = false;

  while (millis() - start < 200) {
    if (spsSerial.available()) {
      uint8_t b = spsSerial.read();
      if (b == 0x7E) {
        if (!inFrame) { inFrame = true; count = 0; continue; }
        else break; // Frame complete
      }
      if (inFrame && count < sizeof(rx)) {
        if (b == 0x7D && spsSerial.available()) { // Handle byte stuffing
          b = spsSerial.read() ^ 0x20;
        }
        rx[count++] = b;
      }
    }
  }

  // Need at least 4 bytes header + 40 bytes float data + 1 byte checksum
  if (count < 45) return false;

  auto getFloat = [](const uint8_t* p) {
    uint32_t val = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
    float f; memcpy(&f, &val, 4);
    return f;
  };

  pm1  = getFloat(&rx[4]);
  pm25 = getFloat(&rx[8]);
  pm10 = getFloat(&rx[16]);
  return true;
}

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- Basic Hardware Test Starting ---");

  // 1. Backlight Setup
  if (PIN_TFT_BL >= 0) {
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH); // Turn backlight full on
  }

  // 2. ST7789 Display Setup
  tft.init(240, 320);
  tft.setRotation(0);
  tft.invertDisplay(false); // Transflective vendor default
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("AIR SENSOR TEST");
  tft.drawFastHLine(0, 32, 240, ST77XX_BLUE);

  // 3. I2C Setup (SCD30 & SGP41)
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);

  // Check SCD30
  if (scd30.begin()) {
    scd30_ok = true;
    scd30.setMeasurementInterval(2);
    Serial.println("[OK] SCD30 detected.");
  } else {
    Serial.println("[FAIL] SCD30 not detected!");
  }

  // Check SGP41 (ping address)
  Wire.beginTransmission(SGP41_ADDR);
  if (Wire.endTransmission() == 0) {
    sgp41_ok = true;
    Serial.println("[OK] SGP41 detected.");
  } else {
    Serial.println("[FAIL] SGP41 not detected!");
  }

  // 4. SPS30 Setup (UART)
  spsSerial.begin(115200, SERIAL_8N1, PIN_SPS30_RX, PIN_SPS30_TX);
  delay(100);
  sps30StartMeasurement();
  sps30_ok = true; // Will verify on first frame read
  Serial.println("[OK] SPS30 initialized on Serial1.");
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  float co2 = NAN, temp = NAN, rh = NAN;
  uint16_t vocRaw = 0, noxRaw = 0;
  float pm1 = NAN, pm25 = NAN, pm10 = NAN;
  bool sgp_read_ok = false;
  bool sps_read_ok = false;

  // 1. Read SCD30
  if (scd30_ok && scd30.dataReady()) {
    if (scd30.read()) {
      co2 = scd30.CO2;
      temp = scd30.temperature;
      rh = scd30.relative_humidity;
    }
  }

  // 2. Read SGP41
  if (sgp41_ok) {
    float tComp = isfinite(temp) ? temp : 25.0f;
    float rhComp = isfinite(rh) ? rh : 50.0f;
    sgp_read_ok = readSGP41(vocRaw, noxRaw, tComp, rhComp);
  }

  // 3. Read SPS30
  if (sps30_ok) {
    sps_read_ok = sps30ReadValues(pm1, pm25, pm10);
  }

  // 4. Print to Serial Monitor
  Serial.println("----------------------------------------");
  if (scd30_ok && isfinite(co2)) {
    Serial.printf("SCD30: CO2=%.0f ppm | Temp=%.1f C | RH=%.1f %%\n", co2, temp, rh);
  } else {
    Serial.println("SCD30: Waiting / No Data");
  }

  if (sgp_read_ok) {
    Serial.printf("SGP41: Raw VOC=%u ticks | Raw NOx=%u ticks\n", vocRaw, noxRaw);
  } else {
    Serial.println("SGP41: Read Error");
  }

  if (sps_read_ok) {
    Serial.printf("SPS30: PM1.0=%.1f | PM2.5=%.1f | PM10=%.1f ug/m3\n", pm1, pm25, pm10);
  } else {
    Serial.println("SPS30: Waiting / Read Error");
  }

  // 5. Direct LCD Render (no memory allocations / double-buffer)
  tft.fillRect(0, 36, 240, 284, ST77XX_BLACK);
  tft.setTextSize(1);
  int y = 42;

  // SCD30 Display Section
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, y); tft.println("--- SCD30 (CO2 & CLIMATE) ---");
  y += 14;
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, y);
  if (isfinite(co2)) tft.printf("CO2:  %.0f ppm", co2);
  else tft.print("CO2:  WAITING...");
  y += 14;
  tft.setCursor(10, y);
  if (isfinite(temp)) tft.printf("Temp: %.1f C  |  RH: %.1f %%", temp, rh);
  else tft.print("Temp/RH: WAITING...");
  y += 24;

  // SGP41 Display Section
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(10, y); tft.println("--- SGP41 (RAW GAS TICKS) ---");
  y += 14;
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, y);
  if (sgp_read_ok) tft.printf("VOC raw: %u", vocRaw);
  else tft.print("VOC raw: ERR / NO DATA");
  y += 14;
  tft.setCursor(10, y);
  if (sgp_read_ok) tft.printf("NOx raw: %u", noxRaw);
  else tft.print("NOx raw: ERR / NO DATA");
  y += 24;

  // SPS30 Display Section
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setCursor(10, y); tft.println("--- SPS30 (PARTICULATE) ---");
  y += 14;
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, y);
  if (sps_read_ok) tft.printf("PM1.0: %.1f ug/m3", pm1);
  else tft.print("PM1.0: WAITING...");
  y += 14;
  tft.setCursor(10, y);
  if (sps_read_ok) tft.printf("PM2.5: %.1f ug/m3", pm25);
  else tft.print("PM2.5: WAITING...");
  y += 14;
  tft.setCursor(10, y);
  if (sps_read_ok) tft.printf("PM10:  %.1f ug/m3", pm10);
  else tft.print("PM10:  WAITING...");
  y += 28;

  // Footer
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, y);
  tft.printf("Uptime: %lu s", millis() / 1000);

  delay(1000); // 1-second refresh rate
}
