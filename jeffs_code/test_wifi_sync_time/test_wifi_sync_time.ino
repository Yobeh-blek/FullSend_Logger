#include <WiFi.h>
#include "time.h"

const char* ssid     = "Cindy6803_2.4GHz";
const char* password = "51245143";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600;
const int   daylightOffset_sec = 0;

bool timeSynced = false;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" Connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP time...");
    delay(500);
  }

  Serial.println("Time synced!");
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");

  // Optional: Turn off WiFi after sync
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  } else {
    Serial.println("Time error");
  }

  delay(1000);
}