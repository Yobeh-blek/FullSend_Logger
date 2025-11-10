#include <SPI.h>
#include <SD.h>

const int chipSelect = 3;  // Change this according to your board

int countFilesInDirectory(File dir) {
    int fileCount = 0;
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break; // No more files
        }
        if (!entry.isDirectory()) {
            fileCount++;  // Count files only, not directories
        }
        entry.close();
    }
    return fileCount;
}

void setup() {
    Serial.begin(115200);
    Serial.println("ok");

    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized.");

    File root = SD.open("/");
    if (!root) {
        Serial.println("Failed to open root directory.");
        return;
    }

    int fileCount = countFilesInDirectory(root);
    Serial.print("Total number of files in SD card: ");
    Serial.println(fileCount);

    root.close();
}

void loop() {
    // Nothing to do in loop
}
