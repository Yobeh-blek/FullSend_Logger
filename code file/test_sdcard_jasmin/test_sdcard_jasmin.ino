#include <SPI.h>
#include <SD.h>
#define SD_SCK   12
#define SD_MISO  13
#define SD_MOSI  11
#define SD_CS    14

SPIClass sdSPI(FSPI);

#define MAX_FILES 10
String fileList[MAX_FILES];
int fileCount = 0;

enum UserState {
  STATE_IDLE,
  STATE_WAIT_READ_INDEX,
  STATE_WAIT_DELETE_INDEX
};

UserState currentState = STATE_IDLE;


void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(11, INPUT_PULLUP); // MOSI
  pinMode(12, INPUT_PULLUP); // SCK
  pinMode(13, INPUT_PULLUP); // MISO
  pinMode(14, INPUT_PULLUP);  // CS

  // Start custom SPI bus
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Mount SD card using custom SPI
  if (!SD.begin(SD_CS, sdSPI, 1000000)) {   // 400kHz safe for level-shifter SD modules
    Serial.println("SD card mount FAILED!");
    return;
  }
  Serial.println("SD card mounted successfully!");
  printMainMenu();
}


void loop() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  Serial.print("Received: ");
  Serial.println(input);

  switch (currentState) {

    case STATE_WAIT_READ_INDEX: {
      int index = input.toInt();
      readSelectedFile(index);
      currentState = STATE_IDLE;
      printMainMenu();
      break;
    }

    case STATE_WAIT_DELETE_INDEX: {
      int index = input.toInt();
      deleteSelectedFile(index);
      currentState = STATE_IDLE;
      printMainMenu();
      break;
    }

    case STATE_IDLE: {
      if (input == "1") {
        createLogFile();
        printMainMenu();
      }
      else if (input == "2") {
        listFilesForReading("/Folder1");
        currentState = STATE_WAIT_READ_INDEX;
      }
      else if (input == "3") {
        listFilesForDeletion("/Folder1");
        currentState = STATE_WAIT_DELETE_INDEX;
      }
      else {
        Serial.println("Invalid option!");
        printMainMenu();
      }
      break;
    }
  }
}


void createLogFile() {
  // ---- CREATE FOLDER 1 ----
  if (!SD.exists("/Folder1")) {
    SD.mkdir("/Folder1");
    Serial.println("Created Folder1");
  }

    // ---- GET UNIQUE FILE NAME ----
  String filePath = getNextLogFilename();
  if (filePath == "") {
    Serial.println("No available filename!");
    return;
  }
  Serial.print("Creating file: ");
  Serial.println(filePath);

  // ---- CREATE Folder1A.csv ----
  File file1A = SD.open(filePath.c_str(), FILE_WRITE);
  if (!file1A) {
    Serial.println("Failed to create file!");
    return;
  }
  if (file1A) {
    file1A.println("year,month,date,hour,minute,seconds_sec,milliseconds_ms,latitude_decimal_deg,longitude_decimal_deg,altitude_m,speed_mps,course_heading_deg,num_satellites,temperature_deg,tire_temp_front_left_deg,tire_temp_front_right_deg,tire_temp_rear_left_deg,tire_temp_rear_right_deg,tire_pressure_front_left_psi,tire_pressure_front_right_psi,tire_pressure_rear_left_psi,tire_pressure_rear_right_psi,imu_accel_x_ms2,imu_accel_y_ms2,imu_accel_z_ms2,raw_imu_accel_x,raw_imu_accel_y,raw_imu_accel_z,gas_pedal_mm,brake_pedal_mm,steering_angle_deg");
    file1A.println("2025,12,24,14,30,15,250,3.1390,101.6869,55.2,12.5,180,10,32.5,45,46,44,45,32,33,31,32,0.12,-0.03,9.81,120,118,102,15,0,5.2");
    file1A.close();
    Serial.println("Created Folder1A.csv inside Folder1");
  }

  // // ---- CREATE FOLDER 2 ----
  // if (!SD.exists("/Folder1/Folder2")) {
  //   SD.mkdir("/Folder1/Folder2");
  //   Serial.println("Created Folder2");
  // }

  // // ---- CREATE Folder2A.csv ----
  // File file2A = SD.open("/Folder1/Folder2/Folder2A.csv", FILE_WRITE);
  // if (file2A) {
  //   file2A.println("Value1,Value2");
  //   file2A.close();
  //   Serial.println("Created Folder2A.csv inside Folder2");
  // }

  Serial.println("All folders and files created successfully!");
}


void listFilesForReading(const char* path) {
  File dir = SD.open(path);
  if (!dir || !dir.isDirectory()) {
    Serial.println("Failed to open directory!");
    return;
  }

  fileCount = 0;
  Serial.println("\nCSV files in Folder1:");

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;

    if (!entry.isDirectory()) {
      String name = String(entry.name());
      if (name.endsWith(".csv") || name.endsWith(".CSV")) {
        if (fileCount < MAX_FILES) {
          fileList[fileCount] = name;
          Serial.printf("[%d] %s\n", fileCount, name.c_str());
          fileCount++;
        }
      }
    }
    entry.close();
  }

  dir.close();

  if (fileCount == 0) {
    Serial.println("No CSV files found.");
    return;
  }
  Serial.println("Enter file number to read:");
}


void readSelectedFile(int index) {
  if (index < 0 || index >= fileCount) {
    Serial.println("Invalid selection!");
    return;
  }

  String fullPath = "/Folder1/" + fileList[index];
  File file = SD.open(fullPath.c_str(), FILE_READ);

  if (!file) {
    Serial.println("Failed to open file for reading!");
    return;
  }

  Serial.println("\nReading: " + fullPath);
  Serial.println("--------------------------------");

  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.println(line);
  }

  file.close();
  Serial.println("------------ End of file ------------\n");
}


void listFilesForDeletion(const char* path) {
  File dir = SD.open(path);
  if (!dir || !dir.isDirectory()) {
    Serial.println("Failed to open directory!");
    return;
  }

  fileCount = 0;
  Serial.println("\nFiles in Folder1:");

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;

    if (!entry.isDirectory()) {
      if (fileCount < MAX_FILES) {
        fileList[fileCount] = String(entry.name());
        Serial.printf("[%d] %s\n", fileCount, entry.name());
        fileCount++;
      }
    }
    entry.close();
  }

  dir.close();

  if (fileCount == 0) {
    Serial.println("No files found.");
    return;
  }
  Serial.println("Enter file number to delete:");
}


void deleteSelectedFile(int index) {
  if (index < 0 || index >= fileCount) {
    Serial.println("Invalid selection!");
    return;
  }

  String fullPath = "/Folder1/" + fileList[index];

  if (SD.remove(fullPath.c_str())) {
    Serial.println("Deleted: " + fullPath);
  } else {
    Serial.println("Delete failed!");
  }
}


String getNextLogFilename() {
  char filename[32];

  for (int i = 1; i <= 999; i++) {
    sprintf(filename, "/Folder1/LOG_%03d.csv", i);

    if (!SD.exists(filename)) {
      return String(filename);
    }
  }

  return ""; // No available filename
}


void printMainMenu() {
  Serial.println();
  Serial.println("========== SD CARD MENU ==========");
  Serial.println("[1] Create CSV file");
  Serial.println("[2] Read CSV file");
  Serial.println("[3] Delete CSV file");
  Serial.println("Enter your choice:");
  Serial.println("==================================");
}
