#include <SPI.h>
#include <SD.h>

#define SD_SCK   12
#define SD_MISO  13
#define SD_MOSI  11
#define SD_CS    14

SPIClass sdSPI(FSPI);   // Use FSPI for custom pins (S3 has FSPI + HSPI)

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(11, INPUT_PULLUP); // MOSI
  pinMode(12, INPUT_PULLUP); // SCK
  pinMode(13, INPUT_PULLUP); // MISO
  pinMode(14, INPUT_PULLUP);  // CS
  Serial.println("Initializing custom SPI SD card...");

  // Start custom SPI bus
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Mount SD card using custom SPI
  if (!SD.begin(SD_CS, sdSPI, 1000000)) {   // 400kHz safe for level-shifter SD modules
    Serial.println("SD card mount FAILED!");
    return;
  }

  Serial.println("SD card mounted successfully!");

  // Test writing a file
  File testFile = SD.open("/test.txt", FILE_WRITE);
  if (testFile) {
    testFile.println("Custom SPI SD OK!");
    testFile.close();
    Serial.println("Write OK");
  } else {
    Serial.println("Write FAILED");
  }

  // Test reading
  testFile = SD.open("/test.txt");
  if (testFile) {
    Serial.println("Read content:");
    while (testFile.available()) {
      Serial.write(testFile.read());
    }
    testFile.close();
  }
}

void loop() {
}
