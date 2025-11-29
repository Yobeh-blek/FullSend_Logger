#include <Wire.h>
#include <U8g2lib.h>

// U8G2_SSD1306_128X64_NONAME_F_HW_I2C(rotation, [reset])
// rotation = U8G2_R0 (normal)
// reset pin = U8X8_PIN_NONE if not used

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,        // no rotation
  U8X8_PIN_NONE   // no reset pin
);

void setup() {
  // Use custom SDA/SCL if needed:
  // Wire.begin(SDA_PIN, SCL_PIN);
  Wire.begin(8, 9);  

  u8g2.begin();
}

void loop() {
  u8g2.clearBuffer();                // Clear internal buffer

  u8g2.setFont(u8g2_font_ncenB08_tr); // Choose a font
  u8g2.drawStr(0, 15, "Hello, Jeff!"); // x=0, y=15

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 32, "U8g2 Example");

  u8g2.setFont(u8g2_font_profont15_tf);
  u8g2.drawStr(0, 50, "ESP32-S3 OLED");

  u8g2.sendBuffer();                  // Display everything
  delay(1000);
}
