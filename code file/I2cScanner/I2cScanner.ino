#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // For ESP32-S3 default: SDA = 8, SCL = 9
  // Change here if using custom pins:
  Wire.begin(8, 9);  // Wire.begin(SDA, SCL);

  Serial.println("I2C Scanner Starting...");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");

  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }    
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("Scan complete\n");

  delay(2000); // scan interval
}
