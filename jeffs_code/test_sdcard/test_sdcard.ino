#include <SPI.h>
#include <SD.h>

// Define SD card SPI pins
#define CS_PIN 3 // Chip Select pin for the SD module

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to start
  Serial.println("Initializing SD card...");
  delay(100);

  // Initialize SD card
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    delay(100);
    return;
  }

  Serial.println("SD card initialized successfully.");

  // Create and write to a file
  File file = SD.open("/example.txt", FILE_WRITE);
  if (file) {
    file.println("Hello, ESP32-C3!");
    file.close();
    Serial.println("Data written to example.txt.");
    delay(100);
  } else {
    Serial.println("Failed to open file for writing.");
    delay(100);
  }

  // Read the file
  file = SD.open("/example.txt");
  if (file) {
    Serial.println("Reading from file:");
    delay(100);
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("Failed to open file for reading.");
    delay(100);
  }
}

void loop() {

}
