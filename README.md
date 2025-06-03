# Indoor Air Quality Monitoring System (AQI-Based IoT Project)

This is a real-time indoor air quality monitoring system based on the Internet of Things (IoT). It uses a NodeMCU ESP8266 microcontroller and multiple environmental sensors to measure and display Air Quality Index (AQI), temperature, humidity, pressure, and gas levels.

## 🌐 Features

- 📊 Real-time monitoring of:
  - Temperature, Humidity, Pressure
  - PM1.0, PM2.5, PM10 (from PMS7003)
  - Gas resistance (from BME680)
  - CO (from MQ-7), VOCs/CO₂ (from MQ135), Flammable gases
  - AQI (calculated based on EPA standards)
- 📟 OLED display for local readout
- 🔊 Buzzer alerts on poor air quality
- 🌍 Web interface hosted on local Wi-Fi for remote monitoring
- ☁️ Firebase integration for data storage and remote access
- 📡 Serial Monitor output for debugging and logging
- 🛠️ Designed for indoor environmental monitoring and research use

## 🧰 Components Used

| Component        | Description                           |
|------------------|---------------------------------------|
| NodeMCU ESP8266  | Main microcontroller (Wi-Fi enabled) |
| BME680           | Temp, Humidity, Pressure, Gas         |
| PMS7003          | PM1.0, PM2.5, PM10 sensor             |
| MQ135            | Air quality sensor (VOCs, CO₂)        |
| MQ-7             | CO gas sensor                         |
| Flammable gas sensor | Mapped 0–1000                     |
| OLED 0.96" I2C   | Data display screen                   |
| Buzzer           | Audio alert device                    |

## 📡 Data Flow

1. Sensor data collected via ESP8266
2. Data sent to:
   - OLED screen (live)
   - Serial monitor
   - Firebase database
   - Local HTML web server
3. AQI calculated based on EPA-style scale (0–500) from:
   - PM1.0 and PM2.5 (PMS7003)
   - MQ135
   - Gas resistance (BME680, mapped to AQI)
4. Final AQI = Maximum of all mapped values

## 📊 AQI Mapping Logic

- **PM1.0/PM2.5**: EPA AQI formula
- **Gas resistance**:
  - >100,000 Ω → AQI 0–50 (Good)
  - 50,000–100,000 Ω → AQI 51–200 (Moderate)
  - <50,000 Ω → AQI 201–500 (Poor to Hazardous)
- **MQ135 & MQ-7**: Scaled based on resistance levels
- **Final AQI** = Max(PM AQI, MQ135 AQI, Gas AQI, CO AQI)

## 🌐 Web Interface

- Built-in ESP8266 web server
- Displays sensor readings and AQI in an attractive HTML page
- Accessible over Wi-Fi on the same network

## 🔔 Buzzer Alerts

- **AQI > 200**: Buzzer sounds to alert hazardous air

## 💾 Firebase Integration

- Sensor data uploaded to Firebase Realtime Database
- Enables mobile/web monitoring and future analytics

## 🛠️ Setup Instructions

1. **Wiring**: Connect sensors to ESP8266 as per pin configuration.
2. **Arduino IDE Setup**:
   - Install ESP8266 board package
   - Install required libraries:
     - `Adafruit_BME680`
     - `Adafruit_Sensor`
     - `PMS`
     - `Adafruit_GFX`, `Adafruit_SSD1306`
     - `Firebase ESP8266`
3. **Configure Firebase & Wi-Fi credentials** in the source code.
4. **Upload code** to ESP8266 via USB.
5. **Power up** the system and monitor data via OLED, Serial, Web, and Firebase.

## 📷 Sample Output (OLED)

Temp: 25.4°C
Humidity: 42%
Pressure: 1013 hPa
PM2.5: 35 µg/m³
MQ135: 325
Gas AQI: 65
Final AQI: 120 (Moderate)


## 👨‍🔬 Applications

- Indoor air quality monitoring (homes, offices)
- Research and academic projects
- Smart home integration
- Environmental awareness campaigns
