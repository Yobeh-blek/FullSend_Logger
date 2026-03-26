#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_BNO055.h>
#include <math.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include "time.h"

#define SDA_PIN 8
#define SCL_PIN 9

#define SD_CS   14
#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK  18

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

SPIClass spiSD(HSPI);
File logFile;
String logFileName;

// ---------- WIFI TIME ----------
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600;
const int   daylightOffset_sec = 0;

struct WiFiNetwork {
  const char* ssid;
  const char* password;
};

// Add multiple WiFi here
WiFiNetwork wifiList[] = {
  {"Cindy6803_2.4GHz", "51245143"},
  {"Jeff Fong", "1234567890"},
  {"SMD_General", "SmdGeneral@2024"}
};

unsigned long lastLog = 0;
const int logInterval = 16; // 60Hz

unsigned long startTime = 0;


// ---------- CREATE FILE ----------
String createFileName() {
  int fileNumber = 1;
  String fileName;

  while (true) {
    fileName = "/LOG" + String(fileNumber) + ".CSV";
    if (!SD.exists(fileName)) {
      break;
    }
    fileNumber++;
  }
  return fileName;
}

// ---------- GET TIME STRING ----------
String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "0000-00-00 00:00:00";
  }

  char timeString[25];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}


bool connectToWiFi() {
  int numNetworks = sizeof(wifiList) / sizeof(wifiList[0]);

  for (int i = 0; i < numNetworks; i++) {
    Serial.print("Trying WiFi: ");
    Serial.println(wifiList[i].ssid);

    WiFi.begin(wifiList[i].ssid, wifiList[i].password);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 10) {
      delay(500);
      Serial.print(".");
      retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to: " + String(wifiList[i].ssid));
      return true;
    }

    Serial.println("\nFailed, trying next WiFi...");
  }

  return false;
}

void syncTimeWithNTP() {
  if (connectToWiFi()) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      Serial.println("Waiting for NTP time...");
      delay(500);
    }

    Serial.println("Time synced:");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  } else {
    Serial.println("No WiFi found, cannot sync time.");
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  // Sensors
  if (!mag.begin()) {
    Serial.println("HMC5883L not detected");
    while (1);
  }

  if (!bno.begin()) {
    Serial.println("BNO055 not detected");
    while (1);
  }

  bno.setExtCrystalUse(true);
  bno.setMode(OPERATION_MODE_NDOF);
  delay(1000);

  // SD Card
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("SD Card Failed");
    while (1);
  }

  Serial.println("SD Card Ready");

  // WiFi Time Sync
  syncTimeWithNTP();
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP time...");
    delay(500);
  }

  Serial.println("Time synced!");
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  startTime = millis();
  // Turn off WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Create log file
  logFileName = createFileName();
  Serial.print("Logging to: ");
  Serial.println(logFileName);

  logFile = SD.open(logFileName, FILE_WRITE);
  if (logFile) {
    logFile.println("time_ms,accX,accY,accZ,magX,magY,magZ,heading,pitch,roll,yaw,gyroX,gyroY,gyroZ");
    logFile.flush();
  }
}


// ---------- LOOP ----------
void loop() {
  
  if (millis() - lastLog >= logInterval) {
    lastLog = millis();
    sensors_event_t mag_event;
    mag.getEvent(&mag_event);

    sensors_event_t accel_event;
    bno.getEvent(&accel_event, Adafruit_BNO055::VECTOR_LINEARACCEL);

    sensors_event_t euler_event;
    bno.getEvent(&euler_event, Adafruit_BNO055::VECTOR_EULER);

    sensors_event_t gyro_event;
    bno.getEvent(&gyro_event, Adafruit_BNO055::VECTOR_GYROSCOPE);

    float yaw   = euler_event.orientation.x;
    float roll  = euler_event.orientation.y;
    float pitch = euler_event.orientation.z;
    float gyroX = gyro_event.gyro.x * DEG_TO_RAD;
    float gyroY = gyro_event.gyro.y * DEG_TO_RAD;
    float gyroZ = gyro_event.gyro.z * DEG_TO_RAD;

    float heading = atan2(mag_event.magnetic.y, mag_event.magnetic.x);
    if (heading < 0) heading += 2 * PI;
    float headingDegrees = heading * 180 / PI;

    

    // Write to SD
    if (logFile) {
      unsigned long timeNow = millis() - startTime;
      logFile.print(timeNow); logFile.print(",");

      logFile.print(accel_event.acceleration.x); logFile.print(",");
      logFile.print(accel_event.acceleration.y); logFile.print(",");
      logFile.print(accel_event.acceleration.z); logFile.print(",");

      logFile.print(mag_event.magnetic.x); logFile.print(",");
      logFile.print(mag_event.magnetic.y); logFile.print(",");
      logFile.print(mag_event.magnetic.z); logFile.print(",");

      logFile.print(headingDegrees); logFile.print(",");
      logFile.print(pitch); logFile.print(",");
      logFile.print(roll); logFile.print(",");
      logFile.print(yaw); logFile.print(",");
      logFile.print(gyroX);logFile.print(",");
      logFile.print(gyroY);logFile.print(",");
      logFile.println(gyroZ);

      logFile.flush();
    }

    // OLED Display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);

    // Heading
    u8g2.setCursor(0, 10);
    u8g2.print("HDG:");
    u8g2.print(headingDegrees, 1);

    // Acceleration XYZ (in G)
    u8g2.setCursor(0, 25);
    u8g2.print("AX:");
    u8g2.print(accel_event.acceleration.x / 9.81, 2);

    u8g2.setCursor(0, 35);
    u8g2.print("AY:");
    u8g2.print(accel_event.acceleration.y / 9.81, 2);

    u8g2.setCursor(0, 45);
    u8g2.print("AZ:");
    u8g2.print(accel_event.acceleration.z / 9.81, 2);

    // Time
    u8g2.setCursor(0, 60);
    u8g2.print(getTimeString().substring(11,19));

    u8g2.sendBuffer();

  }
}