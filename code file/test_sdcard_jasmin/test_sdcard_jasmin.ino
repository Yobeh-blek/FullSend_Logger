#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// Change these to match your wiring
#define SD_CS   10
#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK  12


void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH); // Make sure SD card is idle

  Serial.println("\n=== ESP32-S3 SD diagnostic (fixed SD.begin call) ===");

  // Lower SPI frequency for reliability during testing: 4 MHz
  const uint32_t spiFreq = 4000000;

  // Initialize the default SPI bus and assign pins
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // IMPORTANT: pass SPI (object) not &SPI (pointer)
  Serial.println("Calling SD.begin(CS, SPI, 4MHz) ...");
  if (!SD.begin(SD_CS, SPI, spiFreq)) {
    Serial.println("SD.begin FAILED!");
    return;
  }
  Serial.println("SD.begin OK!");

  Serial.println("=== Diagnostic complete ===");
}

void loop() {
  delay(10000);
}
