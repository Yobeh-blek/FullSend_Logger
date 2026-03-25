#include <Wire.h>
#include <QMC5883LCompass.h>

QMC5883LCompass compass;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);   // SDA, SCL

  compass.init();

  Serial.println("QMC5883L Ready");
}

void loop() {
  compass.read();

  int x = compass.getX();
  int y = compass.getY();
  int z = compass.getZ();
  int azimuth = compass.getAzimuth(); // Heading 0–359°

  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.print(y);
  Serial.print(" Z: ");
  Serial.print(z);
  Serial.print(" Heading: ");
  Serial.println(azimuth);

  delay(100);
}
