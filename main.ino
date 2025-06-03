#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// OLED Settings
#define SCREEN_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Wi-Fi & Firebase
#define WIFI_SSID "Apurav's S23 FE"
#define WIFI_PASSWORD "qwerty1234"
#define FIREBASE_HOST "airqualitymonitor-df0db-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "eoYQGSmlGEIzDPC5FSRxtwAGTbVjmu47uPQHGQeT"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Sensor Pins
#define MQ7_DOUT D5
#define MQ135_AOUT A0
Adafruit_BME680 bme;
SoftwareSerial pmsSerial(D6, D7); // RX, TX

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!bme.begin()) {
    Serial.println("BME680 not found!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setGasHeater(320, 150);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Initializing...");
  display.display();
  delay(2000);
}

// Map gas resistance to AQI-like scale
int mapGasToAQI(uint32_t gasRes) {
  if (gasRes > 100000) {
    return random(0, 51);
  } else if (gasRes > 50000 && gasRes <= 100000) {
    return random(51, 201);
  } else {
    return random(201, 501);
  }
}

// Determine which sensor contributed most to AQI
String getAQISource(int pm25, int pm10, int mq135, int gasVal, int aqi) {
  if (aqi == pm25) return "PM2.5";
  if (aqi == pm10) return "PM1.0";
  if (aqi == mq135) return "MQ135";
  return "Gas";
}

// Determine air quality category
String getAQICategory(int aqi) {
  if (aqi <= 50) return "Good";
  else if (aqi <= 100) return "Moderate";
  else if (aqi <= 200) return "Poor";
  else if (aqi <= 300) return "Dangerous";
  else return "Hazardous";
}

void loop() {
  bme.performReading();
  float temp = bme.temperature;
  float hum = bme.humidity;
  float pres = bme.pressure / 100.0;
  uint32_t gas_res = bme.gas_resistance;

  // PMS7003 Logic
  uint8_t buffer[32];
  int pm25 = -1, pm10 = -1;
  if (pmsSerial.available() >= 32) {
    if (pmsSerial.read() == 0x42) {
      buffer[0] = 0x42;
      buffer[1] = pmsSerial.read();
      for (int i = 2; i < 32; i++) {
        buffer[i] = pmsSerial.read();
      }
      pm25 = buffer[12] << 8 | buffer[13];
      pm10 = buffer[14] << 8 | buffer[15];
    }
  }

  int mq135_val = analogRead(MQ135_AOUT);
  int mq7_val = digitalRead(MQ7_DOUT);

  // Constrain all values to 0–500
  pm25 = constrain(pm25, 0, 500);
  pm10 = constrain(pm10, 0, 500);
  mq135_val = map(mq135_val, 0, 1023, 0, 500);
  mq135_val = constrain(mq135_val, 0, 500);
  int gasVal = mapGasToAQI(gas_res);
  gasVal = constrain(gasVal, 0, 500);

  // Determine final AQI
  int aqi = max(max(pm25, pm10), max(mq135_val, gasVal));
  String aqiSource = getAQISource(pm25, pm10, mq135_val, gasVal, aqi);
  String aqiCategory = getAQICategory(aqi);

  // --- Serial Output ---
  Serial.println("-----------");
  Serial.printf("Temp: %.2f °C\n", temp);
  Serial.printf("Humidity: %.2f %%\n", hum);
  Serial.printf("Pressure: %.2f hPa\n", pres);
  Serial.printf("Gas: %lu Ω\n", gas_res);
  Serial.printf("PM2.5: %d µg/m3\n", pm25);
  Serial.printf("PM1.0: %d µg/m3\n", pm10);
  Serial.printf("MQ135: %d\n", mq135_val);
  Serial.printf("MQ7: %d\n", mq7_val);
  Serial.printf("AQI: %d (%s)\n", aqi, aqiSource.c_str());
  Serial.printf("AirQuality: %s\n", aqiCategory.c_str());

  // --- Firebase Send ---
  FirebaseJson json;
  json.set("temperature", temp);
  json.set("humidity", hum);
  json.set("pressure", pres);
  json.set("gas", gas_res);
  json.set("pm25", pm25);
  json.set("pm10", pm10);
  json.set("mq135", mq135_val);
  json.set("mq7", mq7_val);
  json.set("aqi", aqi);
  json.set("airquality", aqiCategory);

  if (Firebase.setJSON(fbdo, "/airquality/data", json)) {
    Serial.println("Data sent to Firebase");
  } else {
    Serial.println("Firebase send failed: " + fbdo.errorReason());
  }

  // --- OLED Display ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("T: %.0f  H: %.0f%%\n", temp, hum);
  display.printf("P: %.0fhPa\n", pres);
  display.printf("PM2.5:%d PM1.0:%d\n", pm25, pm10);
  display.printf("MQ135:%d MQ7:%d\n", mq135_val, mq7_val);
  display.printf("Gas: %d\n", gasVal);
  display.printf("AQI: %d (%s)\n", aqi, aqiSource.c_str());
  display.printf("air quality:%s\n", aqiCategory.c_str());
  display.display();

  delay(1000); // 1 second
}
