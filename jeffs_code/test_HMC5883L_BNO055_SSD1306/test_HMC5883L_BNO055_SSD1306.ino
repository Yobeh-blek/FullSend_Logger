#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_BNO055.h>
#include <math.h>
#include <SPI.h>
#include <SD.h>

#define SDA_PIN 8
#define SCL_PIN 9

#define SD_CS   14
#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK  18


U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

File logFile;

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

SPIClass spiSD(HSPI);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

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

  // Start SPI
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("SD Card Failed");
    while (1);
  }

  Serial.println("SD Card Ready");

  // Create new file
  String newFileName = createFileName();
  Serial.print("Logging to: ");
  Serial.println(newFileName);

  logFile = SD.open(newFileName, FILE_WRITE);

  if (logFile) {
    logFile.println("time,accX,accY,accZ,magX,magY,magZ,heading,pitch,roll,yaw,gyroZ");
    logFile.flush();
  }
}

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

  float gyroZ = gyro_event.gyro.z; // yaw rate (rad/s)

  float heading = atan2(mag_event.magnetic.y, mag_event.magnetic.x);
  if (heading < 0) heading += 2 * PI;
  float headingDegrees = heading * 180 / PI;

  // Write to SD
  logFile = SD.open("/log.csv", FILE_APPEND);
  if (logFile) {
    logFile.print(millis()); logFile.print(",");

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

  // OLED Display
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

  delay(30);
}