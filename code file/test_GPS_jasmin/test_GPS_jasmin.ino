#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial GPS_Serial(1);

#define GPS_RX_PIN 18   // ESP32 RX  ← GPS TX
#define GPS_TX_PIN 17   // ESP32 TX  → GPS RX
#define GPS_BAUD   9600 // NEO-M8L default
// #define GPS_BAUD 115200 // NEO-M8L default


void setup() {
  Serial.begin(9600);
  Serial.println("ESP32-S3 + NEO-M8L GPS");
  GPS_Serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void loop() {
  while (GPS_Serial.available()>0) {
    char c = GPS_Serial.read();  
    gps.encode(c);               
    Serial.write(c);               
    // If GPS received data should be e.g."$GNGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"
  }
  
  if (gps.location.isValid()) 
  {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    Serial.print("Altitude: ");
    Serial.println(gps.altitude.meters());
  } 
  else 
  {
    Serial.println("Waiting for GPS fix...");
    delay(50000);
  }
}
