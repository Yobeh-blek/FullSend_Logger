#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Create BNO055 object
Adafruit_BNO055 bno = Adafruit_BNO055(55,0x29);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // For ESP32-S3 custom I2C pins (SDA = 8, SCL = 9)
  Wire.begin(8, 9);

  Serial.println("BNO055 Test Starting...");

  if (!bno.begin()) {
    Serial.println("Failed to detect BNO055! Check wiring or I2C address.");
    while (1);
  }

  delay(1000);
  bno.setExtCrystalUse(true);  // Use crystal if available
}

void loop() {
  // --- Orientation data (Euler angles) ---
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  Serial.print("Heading: ");
  Serial.print(euler.x());   // Yaw
  Serial.print("\tRoll: ");
  Serial.print(euler.z());
  Serial.print("\tPitch: ");
  Serial.println(euler.y());

  // --- Acceleration (m/s^2) ---
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  Serial.print("Accel X: ");
  Serial.print(accel.x());
  Serial.print("\tY: ");
  Serial.print(accel.y());
  Serial.print("\tZ: ");
  Serial.println(accel.z());

  // --- Temperature ---
  Serial.print("Temp: ");
  Serial.println(bno.getTemp());

  Serial.println("-------------------------");
  delay(200);
}
