#include <U8x8lib.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <string.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <utility/imumaths.h>
#include <RTClib.h>
#include <strings.h>

#ifdef U8g8_HAVE_HW_SPI
#include <SPI.h>
#endif

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);


static const int RXPin = 16, TXPin = 17;
static const int GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;         
HardwareSerial gpsSerial(1); 

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

/*
  SD card module
*/
#include <SPI.h>
#include <SD.h>
#define CS_PIN 3 // Chip Select pin for the SD module
//String filename = "/getGPSdata.txt";
File file;

unsigned long startMillis = millis();

void setup()
{
  Serial.begin(115200);
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.println("ok");
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    delay(100);
    return;
  }
  // Create or open the file
  file = SD.open("/gps_data.csv", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing!");
    return;
  }
  file.println("Time, Latitude, Longitude, Speed, Sattellites"); // Header for data
  file.close();

  
  Serial.println("SD card initialized successfully.");
  Serial.println("GPS module is initializing...");
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setPowerSave(0);
  u8x8.draw1x2String(4,1,"NANO COM");
  u8x8.draw1x2String(2,4,"RACE DATA LOG");
  u8x8.drawString(6,7,"V0.1");
  delay(1500);
  u8x8.clearDisplay();
}

void loop(void)
{
  u8x8.draw1x2String(0,3,"Searching GPS...");
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
    if (!gps.location.isUpdated()) { 
    }

    else{
      
      displaySDGPSdata();
      u8x8.clearDisplay();
  } 
  }
  delay(100);  
}


void displaySDGPSdata()
{
  if (gps.location.isUpdated()) {
    char lat[20];
    char lng[20];
    char tme[40];
    char kph[12];
    char sat[12];
    unsigned long gpsMillis;
    int hour = (gps.time.hour() + 8) % 24;;
    int minute = gps.time.minute();
    int second = gps.time.second();
    if (gps.time.isUpdated()) {
    unsigned long centisecond = gps.time.centisecond();
    gpsMillis = centisecond + (millis() - startMillis);
    Serial.print("GPS Time (ms precision): ");
    Serial.println(gpsMillis + (millis() - startMillis));
    }
    
    String rawTime = String(hour) + ":" + String(minute) + ":" + String(second)+":"+String(gpsMillis);
    dtostrf(gps.location.lat(),7,7,lat);
    dtostrf(gps.location.lat(),7,7,lng);
    dtostrf(gps.time.value(),6,0,tme);
    dtostrf(gps.speed.kmph(),6,2,kph);
    dtostrf(gps.satellites.value(),6,0,sat);
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    double speedkph = gps.speed.kmph();
    double nofsatellites = gps.satellites.value();
    
    
    file = SD.open("/gps_data.csv", FILE_APPEND);
    if (file) {
      // Write the GPS data
      file.print(rawTime);
      file.print(", ");
      file.print(latitude,7);
      file.print(", ");
      file.print(longitude,7);
      file.print(", ");
      file.print(speedkph,10);
      file.print(", ");
      file.println(nofsatellites,2);

      file.close();
      Serial.println("Data saved to SD card.");
      
      // Update display
      u8x8.drawString(0, 0, "Latitude:");
      u8x8.clearLine(1);
      u8x8.drawString(0, 1, lat);
      u8x8.drawString(0, 2, "Longitude:");
      u8x8.clearLine(3);
      u8x8.drawString(0, 3, lng);
      u8x8.clearLine(4);
      u8x8.clearLine(5);
      u8x8.drawString(0, 4, "Time:");
      u8x8.drawString(0, 5, rawTime.c_str());
      u8x8.clearLine(6);
      u8x8.drawString(0, 6, "km/h:");
      u8x8.drawString(5, 6, kph);
      u8x8.clearLine(7);
      u8x8.drawString(0, 7, "SATE:");
      u8x8.drawString(5, 7, sat);
      delay(100);
    
    } else {
      u8x8.clearDisplay();
      u8x8.drawString(16, 5, "WRITE FAILED");
      delay(50);
    }
   
  }

  else 
  {
    u8x8.clearDisplay();
    u8x8.drawString(32,16,"NO data");
    delay(100);
    u8x8.clearDisplay();
  }
}
