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

  // ---- CREATE FOLDER 1 ----
  if (!SD.exists("/Folder1")) {
    SD.mkdir("/Folder1");
    Serial.println("Created Folder1");
  }

  // ---- CREATE Folder1A.csv ----
  File file1A = SD.open("/Folder1/Folder1A.csv", FILE_WRITE);
  if (file1A) {
    file1A.println("Header1,Header2");  // Example content
    file1A.close();
    Serial.println("Created Folder1A.csv inside Folder1");
  }

  // ---- CREATE FOLDER 2 ----
  if (!SD.exists("/Folder1/Folder2")) {
    SD.mkdir("/Folder1/Folder2");
    Serial.println("Created Folder2");
  }

  // ---- CREATE Folder2A.csv ----
  File file2A = SD.open("/Folder1/Folder2/Folder2A.csv", FILE_WRITE);
  if (file2A) {
    file2A.println("Value1,Value2");
    file2A.close();
    Serial.println("Created Folder2A.csv inside Folder2");
  }

  Serial.println("All folders and files created successfully!");
}

void loop() {
}
