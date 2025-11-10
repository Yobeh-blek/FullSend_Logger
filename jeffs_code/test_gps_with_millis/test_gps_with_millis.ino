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
File file;
String filename;
unsigned long startMillis = millis();
unsigned long centisecond;
int date, month, year,hour,minute,second;
char c ;


//gps configuration
double latitude = 0.0;
double longitude = 0.0;
double speedkph = 0.0;
double previousSpeedkph =0.0;
double nofsatellites = 0.0;

// Button configuration
const int upPin = 2;
const int downPin = 1;
const int selectPin = 0;
const int cancelPin = 10;

int currentPage = 0;    // Current page in the main menu
int featurePage = 0;    // Current feature page
const int totalPageCount = 4;
const int totalFeatureCount = 8;

unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 200;
const unsigned long menuChangeInterval =500;
unsigned long lastMenuChangeTime = 0; 

unsigned long lastGpsUpdateTime = 0; 
const unsigned long gpsUpdateInterval = 500;

bool updatedata = false;
bool inMenuPage = false;  // Tracks whether the user is in the feature menu
double totallaptime =0.00;
double sector1time = 0.00;
double sector2time = 0.00;
double sector3time = 0.00;
double personalbest =0.00;
double personallast =0.00;
double laptime[20] ={};
int lapcount = 0;
double delta= 0.00;
/*
   if negative time = xframe = 64, timedomain = ++;
   if positive time = xframe = 0, timedomain = timedomain - xframe;
*/
int xframe =0;
int timedomain =64 - xframe;

void setup()
{
  Serial.begin(115200);
  /*

    button setup
  
  */
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(selectPin, INPUT_PULLUP);
  pinMode(cancelPin, INPUT_PULLUP);
  
  /*

   Start page
  
  */
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(4,16,"NANO COM");
  u8g2.drawStr(24,40,"Race log");
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(50,64,"V0.1");
  u8g2.sendBuffer();
  delay(1500);
  u8g2.clearDisplay();

  /*

    GPS setup
  
  */
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.clearBuffer();
  u8g2.drawStr(40,32,"INIT...");
  u8g2.drawFrame(0,40,128,16);
  for(int i = 0; i<127; i++)
  {
    u8g2.drawBox(0,40,i,16);
    u8g2.sendBuffer();
  }
  /*

    SD card setup
  
  */
  if (!SD.begin(CS_PIN)) {
    u8g2.clearBuffer();
    u8g2.drawStr(16,8,"SD card not detected!");
    u8g2.sendBuffer();
    delay(1500);
    u8g2.clearDisplay();
    return;
  }
    
//    String file_time = String(year)+"_"+String(month)+"_"+String(date)+"_"+String(hour)+"_"+String(minute);
  String file_time = "GPS_DATA";
  filename = "/"+file_time+".csv";
  if(SD.exists(filename))
  {
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.clearBuffer();
      u8g2.drawStr(0,18,"File created:");
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0,40,file_time.c_str());
      u8g2.sendBuffer();
      delay(1500);
      u8g2.clearDisplay();
  
  }
  
  else{
      file = SD.open(filename, FILE_WRITE);
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.clearBuffer();
      u8g2.drawStr(0,18,"Data save to:");
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0,40,file_time.c_str());
      u8g2.sendBuffer();
      delay(1500);
      u8g2.clearDisplay();
      if(!file)
      {
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.clearBuffer();
        u8g2.drawStr(0,16,"No such file detected");
        u8g2.drawStr(0,24,"Failed to open file");
        u8g2.drawStr(64,33," for writing!");
        u8g2.sendBuffer();
        delay(1500);
        u8g2.clearDisplay();
    
        
      }
  
      else{
        file.println("Time, Latitude, Longitude, Speed, Sattellites"); // Header for data
        file.close();
       
      }
    }
  u8g2.drawStr(0, 12, ">1) Feature");
  u8g2.drawStr(0, 28, "2) New Race");
  u8g2.drawStr(0, 44, "3) Leaderboard");
  u8g2.drawStr(0, 60, "4) Setting");
  u8g2.sendBuffer();
}


void loop()
{
  u8g2.clearBuffer();
   // Handle GPS updates in the background
  if (millis() - lastGpsUpdateTime >= gpsUpdateInterval) {
    lastGpsUpdateTime = millis();
    while (gpsSerial.available() > 0) {
      char c = gpsSerial.read();
      gps.encode(c);
    }

    // Update GPS data here (if needed)
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      speedkph = gps.speed.kmph();
      nofsatellites = gps.satellites.value();

      // Log data to SD card (if desired)
      if (file) {
        file = SD.open(filename, FILE_APPEND); 
        if (file) {
          file.print(gps.time.value()); 
          file.print(",");
          file.print(latitude, 6); 
          file.print(",");
          file.print(longitude, 6); 
          file.print(",");
          file.print(speedkph, 2); 
          file.print(",");
          file.println(nofsatellites);
          file.close();
     }
    }
   }
  }
  Serial.print("Current Page: "); Serial.println(currentPage);
  Serial.print("Feature Page: "); Serial.println(featurePage);
  Serial.print("Speed: "); Serial.println(speedkph);

  selectpage();
  if (inMenuPage = true) {
    menupage(featurePage);
  }
  else
  {
    drawPage(currentPage);
  }
  
}


void selectpage()
{
  unsigned long currentMillis = millis();
  // Handle buttons
  if (digitalRead(upPin) == LOW && currentMillis - lastButtonPressTime > debounceDelay) {
    lastButtonPressTime = currentMillis;
    
    if (inMenuPage = false) {
      featurePage = (featurePage + 1) % totalFeatureCount;
      
    } else {
      currentPage = (currentPage + 1) % totalPageCount;
    }
   
    
  }

  if (digitalRead(downPin) == LOW && currentMillis - lastButtonPressTime > debounceDelay) {
    lastButtonPressTime = currentMillis;
    if (inMenuPage = false) {
      featurePage = (featurePage + 1) % totalFeatureCount;
      
    } else {
      currentPage = (currentPage + 1) % totalPageCount;
   
    }
  }

  if (digitalRead(selectPin) == LOW && currentMillis - lastButtonPressTime > debounceDelay) {
    lastButtonPressTime = currentMillis;
    inMenuPage = true;
  }

  if (digitalRead(cancelPin) == LOW && currentMillis - lastButtonPressTime > debounceDelay) {
    lastButtonPressTime = currentMillis;
    inMenuPage = false;
  }
  
  
}

void drawPage(int page) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  switch (page) {
    case 0:
      u8g2.drawStr(0, 12, ">1) Feature");
      u8g2.drawStr(0, 28, "2) New Race");
      u8g2.drawStr(0, 44, "3) Leaderboard");
      u8g2.drawStr(0, 60, "4) Setting");
      break;
    case 1:
      u8g2.drawStr(0, 12, "1) Feature");
      u8g2.drawStr(0, 28, ">2) New Race");
      u8g2.drawStr(0, 44, "3) Leaderboard");
      u8g2.drawStr(0, 60, "4) Setting");
      break;
    case 2:
      u8g2.drawStr(0, 12, "1) Feature");
      u8g2.drawStr(0, 28, "2) New Race");
      u8g2.drawStr(0, 44, ">3) Leaderboard");
      u8g2.drawStr(0, 60, "4) Setting");
      break;
    case 3:
      u8g2.drawStr(0, 12, "1) Feature");
      u8g2.drawStr(0, 28, "2) New Race");
      u8g2.drawStr(0, 44, "3) Leaderboard");
      u8g2.drawStr(0, 60, ">4) Setting");
      break;
  }
  u8g2.sendBuffer();
}

void menupage(int page) {
  u8g2.clearBuffer();
  switch (page) {
    case 0: // speed
      page1();
      break;
    case 1: // lap time
      page2();
      break;
    case 2: //delta time
      page3();
      break;
      
    case 3: //best/last time
      page4();
      break;
    case 4:// G meter
      page5();
      break;
      
    case 5://tire temp
      page6();
      break;
    case 6: // sector time
      page7();
      break;
    case 7: //total time
      page8();
      break;
    default:
      
      break;
  }
  u8g2.sendBuffer();
}




void page1()
{
  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.drawStr(28, 26, String(speedkph).c_str());
  u8g2.drawStr(44, 64, " kmh");
  u8g2.sendBuffer();
}

void page2()
{
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.drawStr(0, 12, "1)");
  u8g2.drawStr(0, 26, "2)");
  u8g2.drawStr(0, 38, "3)");
  u8g2.sendBuffer();
}

void page3()
{
  /*  

    if negative time = xframe = 64, timedomain = ++;
    if positive time = xframe = 0, timedomain = timedomain - xframe;
  
  */ 
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(30, 16,"Delta :");
  u8g2.drawFrame(0,32,128,15); // time frame
  u8g2.drawBox(xframe,32,64-xframe,15); // - time domain
  u8g2.drawStr(50, 64, String(delta).c_str());
  u8g2.sendBuffer();
}

void page4()
{
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(0, 24, "Best: ");
  u8g2.drawStr(54, 24,String(personalbest).c_str());
  u8g2.drawStr(0, 50, "Last: ");
  u8g2.drawStr(54, 50,String(personallast).c_str());
  u8g2.sendBuffer();
}

void page5()
{
  u8g2.drawCircle(126/2, 62/2,40, U8G2_DRAW_ALL);
  u8g2.drawCircle(126/2, 62/2,20, U8G2_DRAW_ALL);
  u8g2.drawCircle(126/2, 62/2,2, U8G2_DRAW_ALL);
  u8g2.sendBuffer();
}

void page6()
{
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(10, 10, "FL: ");
  u8g2.drawStr(10, 45, "RL:");
  u8g2.drawStr(70, 10, "FR: ");
  u8g2.drawStr(74, 45, "RR:");
  u8g2.sendBuffer();
}

void page7()
{
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(0, 14, "Sector time :");
  u8g2.drawStr(0, 32, "S1 :");
  u8g2.drawStr(40, 32, String(sector1time).c_str());
  u8g2.drawStr(0, 48, "S2 :");
  u8g2.drawStr(40, 48, String(sector1time).c_str());
  u8g2.drawStr(0, 64, "S3 :");
  u8g2.drawStr(40, 64, String(sector1time).c_str());
  u8g2.sendBuffer();
}

void page8()
{
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(4, 14, "Total time: ");
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.drawStr(4, 38, String(totallaptime).c_str());
  u8g2.drawStr(0, 64, " Seconds");
  u8g2.sendBuffer();
}
