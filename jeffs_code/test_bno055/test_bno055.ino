#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// Create BNO055 object
Adafruit_BNO055 bno = Adafruit_BNO055(55,0x29);

void setup() {
  Serial.begin(115200);
  Serial.println("BNO055 Sensor Test");
  Wire.begin(8, 9);  // SDA = GPIO 8, SCL = GPIO 9
  // Initialize the sensor
  if (!bno.begin()) {
    Serial.println("Error: Could not find a BNO055 sensor!");
    while (1); // Stop execution if the sensor is not found
  }

  // Set sensor to normal operation mode
  bno.setExtCrystalUse(true);

  delay(1000);
}

void loop() {
  // Get orientation data (Euler angles)
  uint8_t system, gyro, accel, mag;  
  sensors_event_t event;
  bno.getEvent(&event);
  bno.getCalibration(&system, &gyro, &accel, &mag);
   // Print calibration status
  Serial.print("CALIBRATION -> ");
  Serial.print("SYS: "); Serial.print(system, DEC);
  Serial.print(" GYR: "); Serial.print(gyro, DEC);
  Serial.print(" ACC: "); Serial.print(accel, DEC);
  Serial.print(" MAG: "); Serial.println(mag, DEC);

  // Wait until fully calibrated
  if (system == 3 && gyro == 3 && accel == 3 && mag == 3) {
    Serial.println("Fully Calibrated!");
    // Save calibration data (optional)
    saveCalibration();
  }

  delay(500); // Wait before updating
  // Get linear acceleration in m/s²
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  Serial.print("X: "); Serial.print(euler.x());
  Serial.print(" Y: "); Serial.print(euler.y());
  Serial.print(" Z: "); Serial.println(euler.z());
  delay(10);
}

void saveCalibration() {
  adafruit_bno055_offsets_t offsets;
  bno.getSensorOffsets(offsets);
  
  
  Serial.println("Calibration data:");
  Serial.print("Accel Offset X: "); Serial.println(offsets.accel_offset_x);
  Serial.print("Accel Offset Y: "); Serial.println(offsets.accel_offset_y);
  Serial.print("Accel Offset Z: "); Serial.println(offsets.accel_offset_z);

  // Save offsets to EEPROM/SD card for later use
}
