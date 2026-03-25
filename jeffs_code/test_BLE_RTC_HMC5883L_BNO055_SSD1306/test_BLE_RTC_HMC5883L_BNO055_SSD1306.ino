#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_BNO055.h>
#include <math.h>
#include <SPI.h>
#include <SD.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "time.h"

#define SDA_PIN 8
#define SCL_PIN 9

#define SD_CS   14
#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK  18

// BLE UUID
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pCharacteristic;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

SPIClass spiSD(HSPI);
File logFile;
String logFileName;

// ============ BLE TIME CALLBACK ============
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue().c_str();

    if (rxValue.length() > 0) {
      Serial.println("Received Time: " + rxValue);

      int year, month, day, hour, minute, second;
      sscanf(rxValue.c_str(), "%d-%d-%d %d:%d:%d",
             &year, &month, &day, &hour, &minute, &second);

      struct tm t;
      t.tm_year = year - 1900;
      t.tm_mon  = month - 1;
      t.tm_mday = day;
      t.tm_hour = hour;
      t.tm_min  = minute;
      t.tm_sec  = second;

      time_t newTime = mktime(&t);
      struct timeval now = { .tv_sec = newTime };
      settimeofday(&now, NULL);

      Serial.println("Time set successfully");
    }
  }
};

// ============ CREATE FILE ============
String createFileName() {
  int fileNumber = 1;
  String fileName;

  while (true) {
    fileName = "/LOG" + String(fileNumber) + ".CSV";
    if (!SD.exists(fileName)) break;
    fileNumber++;
  }
  return fileName;
}

// ============ SETUP ============
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

  logFileName = createFileName();
  Serial.print("Logging to: ");
  Serial.println(logFileName);

  logFile = SD.open(logFileName, FILE_WRITE);
  if (logFile) {
    logFile.println("date,time,accX,accY,accZ,magX,magY,magZ,heading,pitch,roll,yaw,gyroZ");
    logFile.flush();
  }

  // BLE Time Sync
  BLEDevice::init("ESP32-TimeSync");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("Waiting for phone to sync time...");
}

// ============ LOOP ============
void loop() {
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
  float gyroZ = gyro_event.gyro.z;

  float heading = atan2(mag_event.magnetic.y, mag_event.magnetic.x);
  if (heading < 0) heading += 2 * PI;
  float headingDegrees = heading * 180 / PI;

  // Get RTC time
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char dateStr[20];
  char timeStr[20];
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

  // Write to SD
  logFile = SD.open(logFileName, FILE_APPEND);
  if (logFile) {
    logFile.print(dateStr); logFile.print(",");
    logFile.print(timeStr); logFile.print(",");

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
    logFile.println(gyroZ);

    logFile.flush();
  }

  // OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.setCursor(0, 10);
  u8g2.print("HDG:");
  u8g2.print(headingDegrees, 1);

  u8g2.setCursor(0, 25);
  u8g2.print("GX:");
  u8g2.print(accel_event.acceleration.x / 9.81);

  u8g2.setCursor(0, 35);
  u8g2.print("GY:");
  u8g2.print(accel_event.acceleration.y / 9.81);

  u8g2.setCursor(0, 45);
  u8g2.print("GZ:");
  u8g2.print(accel_event.acceleration.z / 9.81);

  u8g2.sendBuffer();

  delay(50);
}
