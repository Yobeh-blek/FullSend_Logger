#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  // send packet
  LoRa.beginPacket();
  LoRa.print("Hello World");
  LoRa.endPacket();

  delay(1000);
}
