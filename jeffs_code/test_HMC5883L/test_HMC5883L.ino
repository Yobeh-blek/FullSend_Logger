#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <math.h>

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ESP32 I2C pins
  Wire.begin(8,9);   // SDA, SCL

  Serial.println("HMC5883L Test");

  if(!mag.begin()) {
    Serial.println("No HMC5883L detected. Check wiring!");
    while(1);
  }

  // Set gain (important)
  mag.setMagGain(HMC5883_MAGGAIN_1_3);

  Serial.println("HMC5883L Initialized");
}

void loop() {
  sensors_event_t event;
  mag.getEvent(&event);

  // Print RAW data
  Serial.print("X: ");
  Serial.print(event.magnetic.x);
  Serial.print("  Y: ");
  Serial.print(event.magnetic.y);
  Serial.print("  Z: ");
  Serial.print(event.magnetic.z);

  // Calculate heading
  float heading = atan2(event.magnetic.y, event.magnetic.x);

  // Magnetic declination (Malaysia ≈ 0.22 rad)
  float declinationAngle = 0.22;
  heading += declinationAngle;

  // Normalize to 0–360°
  if (heading < 0)
    heading += 2 * PI;
  if (heading > 2 * PI)
    heading -= 2 * PI;

  float headingDegrees = heading * 180 / PI;

  Serial.print("  Heading: ");
  Serial.println(headingDegrees);

  delay(100);
}
