#include <U8g2lib.h>
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

#ifdef U8g2_HAVE_HW_SPI
#include <SPI.h>
#endif

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


const char* myArray[] = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
const int numItems = sizeof(myArray) / sizeof(myArray[0]);

void setup() {
  u8g2.begin();
  u8g2.clearDisplay(); 
}

void loop() {
  u8g2.setFont(u8g2_font_helvR08_tf); 
  u8g2.drawStr(0, 10, myArray[0]); 
  for(int i = 1; i <6;i++)
  {
    u8g2.drawStr(0, 10*i, myArray[i]); 
    u8g2.sendBuffer();
    delay(1000);
  }
  
}
