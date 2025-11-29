#include <TinyGPSPlus.h>

TinyGPSPlus gps;

const int RX_PIN = 18; 
const int TX_PIN = 17; 
const unsigned long GPS_BAUD = 9600;

unsigned long lastPrint = 0;
const unsigned long PRINT_INTERVAL_MS = 1000;

// void setup() {
//   Serial.begin(115200);
//   delay(100);
//   Serial.println("\nESP32 GPS reader starting...");

//   // Use Serial2 for hardware UART on ESP32
//   Serial2.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

//   Serial.println("Serial2 started. Waiting for GPS data...");
// }

// void loop() {
//   // Read incoming bytes from GPS and feed TinyGPSPlus
//   while (Serial2.available()) {
//     Serial.println("Serial2 RETURN DATA");
//     char c = Serial2.read();
    
//     // Forward raw NMEA to Serial monitor so you can see all sentences
//     Serial.write(c); // raw NMEA output

//     // Feed parser
//     gps.encode(c);
//   }

//   // Periodically print parsed (human-friendly) fields
//   if (millis() - lastPrint >= PRINT_INTERVAL_MS) {
//     lastPrint = millis();
//     printParsed();
//   }
// }

// void printParsed() {
//   Serial.println(F("\n--- Parsed GPS Data ---"));

//   // Location
//   if (gps.location.isValid()) {
//     Serial.print(F("Latitude:  "));
//     Serial.println(gps.location.lat(), 6);
//     Serial.print(F("Longitude: "));
//     Serial.println(gps.location.lng(), 6);
//     Serial.print(F("Fix Age (ms): "));
//     Serial.println(gps.location.age());
//   } else {
//     Serial.println(F("Location: INVALID"));
//   }

//   // Date & Time (UTC)
//   if (gps.date.isValid()) {
//     Serial.print(F("Date (UTC): "));
//     Serial.print(gps.date.month());
//     Serial.print(F("/"));
//     Serial.print(gps.date.day());
//     Serial.print(F("/"));
//     Serial.println(gps.date.year());
//   } else {
//     Serial.println(F("Date: INVALID"));
//   }

//   if (gps.time.isValid()) {
//     Serial.print(F("Time (UTC): "));
//     // Print leading zeros
//     if (gps.time.hour() < 10) Serial.print('0');
//     Serial.print(gps.time.hour());
//     Serial.print(':');
//     if (gps.time.minute() < 10) Serial.print('0');
//     Serial.print(gps.time.minute());
//     Serial.print(':');
//     if (gps.time.second() < 10) Serial.print('0');
//     Serial.print(gps.time.second());
//     Serial.print('.');
//     Serial.println(gps.time.centisecond());
//   } else {
//     Serial.println(F("Time: INVALID"));
//   }

//   // Fix quality / satellites / altitude
//   if (gps.altitude.isValid()) {
//     Serial.print(F("Altitude (meters): "));
//     Serial.println(gps.altitude.meters());
//   } else {
//     Serial.println(F("Altitude: INVALID"));
//   }

//   if (gps.satellites.isValid()) {
//     Serial.print(F("Satellites: "));
//     Serial.println(gps.satellites.value());
//   } else {
//     Serial.println(F("Satellites: INVALID"));
//   }

//   // Speed & course
//   if (gps.speed.isValid()) {
//     Serial.print(F("Speed (knots): "));
//     Serial.println(gps.speed.knots());
//     Serial.print(F("Speed (km/h): "));
//     Serial.println(gps.speed.kmph());
//   } else {
//     Serial.println(F("Speed: INVALID"));
//   }

//   if (gps.course.isValid()) {
//     Serial.print(F("Course (degrees): "));
//     Serial.println(gps.course.deg());
//   } else {
//     Serial.println(F("Course: INVALID"));
//   }

//   // HDOP (horizontal dilution of precision) if available
//   if (gps.hdop.isValid()) {
//     Serial.print(F("HDOP: "));
//     Serial.println(gps.hdop.hdop());
//   } else {
//     Serial.println(F("HDOP: INVALID"));
//   }

// }






void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 17, 18);
}

void loop() {
  if (Serial2.available()) {
    char c = Serial2.read();
    Serial.write(c); // Should see NMEA sentences
  }
}
