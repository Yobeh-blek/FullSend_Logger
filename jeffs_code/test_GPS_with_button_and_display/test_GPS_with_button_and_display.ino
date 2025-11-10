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


static const int RXPin = 20, TXPin = 21;
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

/*

  Buttons

*/
const int k1 = 0;
const int k2 = 1;
const int k3 = 2;
const int k4 = 10;





void setup() {
  Serial.begin(115200);
  
  

}

void loop() {
 

}
