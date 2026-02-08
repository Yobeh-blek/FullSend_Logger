#include <TinyGPSPlus.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <U8g2lib.h>

#define SD_CS_PIN 14

File logFile;
// GPS parser
TinyGPSPlus gps;

// UART1
HardwareSerial GPS(1);

#define GPS_RX_PIN 16   // GPS TX -> ESP32 RX
#define GPS_TX_PIN 17   // GPS RX -> ESP32 TX

static const int GPSBaud = 9600;  // safe for 5 Hz

// Factory reset command
const uint8_t factoryReset[] = {
  0xB5, 0x62,
  0x06, 0x09,
  0x0D, 0x00,
  0xFF, 0xFF, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0x00, 0x00,
  0x17
};

// UBX-CFG-RATE → 1 Hz (1000 ms)
const uint8_t setRate1Hz[] = {
  0xB5, 0x62,       // UBX header
  0x06, 0x08,       // CFG-RATE
  0x06, 0x00,       // payload length
  0xE8, 0x03,       // measRate = 1000 ms (1 Hz)
  0x01, 0x00,       // navRate = 1
  0x01, 0x00,       // timeRef = UTC
  0x01, 0x39        // checksum
};

// UBX-CFG-RATE → 5 Hz (200 ms)
const uint8_t setRate5Hz[] = {
  0xB5, 0x62,
  0x06, 0x08,
  0x06, 0x00,
  0xC8, 0x00,   // 200 ms = 5 Hz
  0x01, 0x00,
  0x01, 0x00,
  0xDE, 0x6A
};

// UBX-CFG-RATE → 10 Hz (100 ms)
const uint8_t setRate10Hz[] = {
  0xB5, 0x62,
  0x06, 0x08,
  0x06, 0x00,
  0x64, 0x00,   // 100 ms = 10 Hz
  0x01, 0x00,
  0x01, 0x00,
  0x7A, 0x12
};

const uint8_t UBX_SET_BAUD_115200[] = {
  0xB5, 0x62,       // UBX sync chars
  0x06, 0x00,       // CFG-PRT
  0x14, 0x00,       // payload length (20 bytes)
  0x01,             // portID = UART1
  0x00,             // reserved
  0x00, 0x00,       // txReady
  0xD0, 0x08, 0x00, 0x00, // mode (8N1)
  0x00, 0xC2, 0x01, 0x00, // baudrate = 115200
  0x07, 0x00,       // inProtoMask (UBX + NMEA)
  0x03, 0x00,       // outProtoMask (UBX + NMEA)
  0x00, 0x00,       // flags
  0x00, 0x00,       // reserved
  0xC0, 0x7E        // checksum
};


// SSD1309 I2C (no reset pin)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  /* reset=*/ U8X8_PIN_NONE
);
#define SDA_PIN 8
#define SCL_PIN 9


void setup()
{
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);   // IMPORTANT
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 32, "SSD1306 OK");
  u8g2.sendBuffer();

  // Start GPS at default baud
  GPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(500);

  // Switch GPS baud to 115200
  GPS.write(UBX_SET_BAUD_115200, sizeof(UBX_SET_BAUD_115200));
  GPS.flush();
  delay(300);

  GPS.end();
  delay(200);
  GPS.begin(115200, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(300);

  // Set update rate to 5 Hz
//  GPS.write(setRate5Hz, sizeof(setRate5Hz));
  GPS.write(setRate10Hz, sizeof(setRate10Hz));
  GPS.flush();
  delay(100);

  // ---------- SPI + SD INIT ----------
  SPI.begin(18, 13, 11, SD_CS_PIN);  
  // SPI.begin(SCK, MISO, MOSI, CS)
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card Mount Failed");
  } else {
    Serial.println("SD Card Ready");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 32, "SD OK");
    u8g2.sendBuffer();
    delay(100);
  }


  // Create / open log file
  logFile = createNewLogFile();
  if (!logFile) {
    Serial.println("Failed to create log file");
    while (1);
  }

  // Write CSV header only once
  if (logFile.size() == 0) {
    logFile.println("date,time,lat,lon,speed_kmph");
    logFile.flush();
  }

  
  
  Serial.println("GPS running @115200, 10Hz");
}

void loop()
{
  while (GPS.available()>0)
  {
    gps.encode(GPS.read());
  }

  static uint32_t last = millis();
  if (GPS.available()) {
    GPS.read();
    if (millis() - last >= 100) { // 10 Hz logging
      displayInfo();
      last = millis();
    }
  }

  // Debug only if truly no data
  static uint32_t lastData = millis();
  if (gps.charsProcessed() > 0)
    lastData = millis();

  if (millis() - lastData > 5000)
  {
    Serial.println("No GPS data (check antenna / sky view)");
    lastData = millis();
  }
}

File createNewLogFile() {
  char filename[20];

  for (int i = 1; i < 1000; i++) {
    sprintf(filename, "/gps_%03d.csv", i);
    if (!SD.exists(filename)) {
      Serial.print("Creating file: ");
      Serial.println(filename);
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 12, filename);

      File file = SD.open(filename, FILE_WRITE);
      if (file) {
        file.println("date,time,lat,lon,speed_kmph");
        file.flush();
        return file;
      }
    }
  }

  Serial.println("No available filename");
  return File();
}

void displayInfo()
{
  if (!gps.location.isValid() || !gps.time.isValid() || !gps.date.isValid())
    return;

  char dateStr[16];
  char timeStr[16];

  sprintf(dateStr, "%04d-%02d-%02d",
          gps.date.year(),
          gps.date.month(),
          gps.date.day());

  sprintf(timeStr, "%02d:%02d:%02d.%02d",
          gps.time.hour(),
          gps.time.minute(),
          gps.time.second(),
          gps.time.centisecond());

  double lat = gps.location.lat();
  double lon = gps.location.lng();
  double speed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;

  // ---- Serial debug ----
  Serial.printf("%s %s, %.7f, %.7f, %.2f km/h\n",
                dateStr, timeStr, lat, lon, speed);

  // ---- SD logging ----
  logFile.printf("%s,%s,%.7f,%.7f,%.2f\n",
                 dateStr, timeStr, lat, lon, speed);

  logFile.flush();   // ensure data is written (safe for 10Hz)
  // ---- OLED display ----
  drawSpeed(speed);
}

void drawSpeed(double speedKmph)
{
  char buf[10];
  sprintf(buf, "%.1f", speedKmph);

  u8g2.clearBuffer();

  // Title
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 12, "Speed");

  // Big speed number
  u8g2.setFont(u8g2_font_logisoso32_tf);
  u8g2.drawStr(0, 55, buf);

  // Unit
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(95, 55, "km/h");

  u8g2.sendBuffer();
}
