#include <TinyGPSPlus.h>

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

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Start GPS at default 9600
  GPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); // RX, TX pins
  delay(500);

  // Send UBX command to change baud rate
  GPS.write(UBX_SET_BAUD_115200, sizeof(UBX_SET_BAUD_115200));
  GPS.flush();
  delay(200);

  // Restart UART at 115200
  GPS.end();
  delay(200);
  GPS.begin(115200, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  // Set update rate (5 Hz recommended)
  GPS.write(setRate5Hz, sizeof(setRate5Hz));
  GPS.flush();
  delay(100);

  Serial.println("GPS switched to 115200 @5Hz");
}

void loop()
{
  while (GPS.available()>0)
  {
    gps.encode(GPS.read());
  }

  if (gps.location.isUpdated())
  {
    displayInfo();
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


void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 7);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 7);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
