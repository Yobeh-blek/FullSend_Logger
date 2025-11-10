#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  tft.init();
  tft.setRotation(1); // Adjust as needed (0-3)
  tft.fillScreen(TFT_BLACK); // Clear the screen

  tft.setTextColor(TFT_WHITE, TFT_BLACK); // White text on black background
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.println("ST7796 Test");
  tft.setCursor(10, 40);
  tft.println("Hello, ESP32-C3!");
}

void loop() {
  // Add any animation or dynamic data updates here
}
