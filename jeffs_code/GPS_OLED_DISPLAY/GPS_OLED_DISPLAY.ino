#include <U8x8lib.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <string.h>

#define GPS_RX_PIN 20  
#define GPS_TX_PIN 21  

TinyGPSPlus gps;         
HardwareSerial gpsSerial(1); 

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);         

void setup(void)
{ 
  Serial.begin(115200);                                       
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS module is initializing...");
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setPowerSave(0);
  u8x8.drawString(0,1,"OK");
  delay(1500);
  u8x8.clearDisplay();
}

void loop(void)
{
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (gps.encode(c)) { 
      displayGPSData();
    }
  }
  delay(1000);  
}


void displayGPSData() {
  if (gps.location.isUpdated()) {
    char lat[20];
    char lng[20]; 
    dtostrf(gps.location.lat(),6,6,lat);
    dtostrf(gps.location.lat(),6,6,lng);
    u8x8.clearDisplay();
    u8x8.drawString(0,1,"Latitude:");
    u8x8.drawString(0,2,lat);
    u8x8.drawString(0,3,"Longitude:");
    u8x8.drawString(0,4,lng);
    delay(500);
    u8x8.clearDisplay();
  }

  else 
  {
    u8x8.clearDisplay();
    u8x8.drawString(0,1,"NO data");
    delay(500);
    u8x8.clearDisplay();
  }
}
