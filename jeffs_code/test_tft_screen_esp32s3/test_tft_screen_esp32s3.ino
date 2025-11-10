#include <Arduino_GFX_Library.h>
#include <SPI.h>

// === Pin Definitions ===
#define TFT_CS   14
#define TFT_DC   12
#define TFT_RST  13
#define TFT_BL   8  // Backlight control (optional)
#define TFT_MOSI 11
#define TFT_MISO 10
#define TFT_CLK  9

// === Initialize Display ===
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_CLK /* SCK */, TFT_MOSI /* MOSI */, TFT_MISO /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7796(bus, TFT_RST, 1 /* rotation */, false /* IPS */);

void setup()
{
  Serial.begin(115200);
  Serial.println("ST7796 with ESP32-S3 test");

  // Initialize the display
  gfx->begin();
  gfx->begin(40000000); // 80 MHz SPI
  gfx->fillScreen(BLACK);

  // Turn on backlight (if controlled by pin)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Draw some graphics
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(20, 20);
  gfx->println("Hello, ST7796!");

  gfx->drawRect(10, 10, 100, 50, RED);
  gfx->fillCircle(150, 120, 30, BLUE);
}

void loop()
{
  gfx->setCursor(20, 60);
  gfx->setTextColor(random(0xFFFF));
  gfx->println(millis());
  delay(500);
}
