#include <Arduino.h>
#include <NimBLEDevice.h>
#include "time.h"

bool timeSynced = false;

void syncTime(NimBLERemoteCharacteristic* chr) {
  std::string value = chr->readValue();

  if (value.length() >= 7) {
    uint16_t year = value[0] | (value[1] << 8);
    uint8_t month = value[2];
    uint8_t day = value[3];
    uint8_t hour = value[4];
    uint8_t minute = value[5];
    uint8_t second = value[6];

    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;

    time_t epoch = mktime(&t);
    struct timeval now = { .tv_sec = epoch };
    settimeofday(&now, NULL);

    Serial.println("Time synced from phone!");
    timeSynced = true;
  }
}

void syncTimeFromPhone() {
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setActiveScan(true);

  Serial.println("Scanning for phone...");
  scan->start(5);

  NimBLEScanResults results = scan->getResults();

  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice* device = results.getDevice(i);

    NimBLEClient* client = NimBLEDevice::createClient();

    if (client->connect(device)) {
      Serial.println("Connected to device");

      NimBLERemoteService* service = client->getService("1805");
      if (service) {
        Serial.println("Found CTS");

        NimBLERemoteCharacteristic* chr = service->getCharacteristic("2A2B");
        if (chr) {
          syncTime(chr);
          client->disconnect();
          return;
        }
      }

      client->disconnect();
    }
  }

  Serial.println("CTS not found");
}

void startBLEServer() {
  NimBLEServer *pServer = NimBLEDevice::createServer();
  NimBLEService *pService = pServer->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("ESP32 Logger");
  pAdvertising->addServiceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
  pAdvertising->start();

  Serial.println("BLE Logger Ready");
}

void setup() {
  Serial.begin(115200);

  setenv("TZ", "MYT-8", 1);
  tzset();

  NimBLEDevice::init("ESP32S3_test");
  NimBLEServer *pServer = NimBLEDevice::createServer(); 
  NimBLEService *pService = pServer->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising(); 
  pAdvertising->setName("ESP32S3_test"); 
  pAdvertising->addServiceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); 
  pAdvertising->start();


  // Step 1: Sync time from phone
  syncTimeFromPhone();

  // Step 2: Start BLE server for logger
  startBLEServer();
}

void loop() {
  if (timeSynced) {
    time_t now = time(nullptr);
    Serial.println(ctime(&now));
  } else {
    Serial.println("Time not synced");
  }

  delay(1000);
}