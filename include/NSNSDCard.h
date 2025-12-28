#pragma once

#include "ClockSettings.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ArduinoJson.h>

#define SCK  18
#define MISO  19
#define MOSI  23
#define CS  5

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}
/*
struct ClockSettings {
    char WifiSSID[64];
    char WifiPWD[64];
    char BlynkKey[32];
    int ClockMode;
    char urlActueleWind[64];
    char url[64];
    int KnopenMin;
};
*/
// Loads the configuration from a file
ClockSettings LoadClockSettings (const char *filename, ClockSettings &settings) {

    SPIClass spi = SPIClass(VSPI);
    spi.begin(SCK, MISO, MOSI, CS);

    if (!SD.begin(CS,spi,80000000)) {
      Serial.println("Card Mount Failed");
      spi.end();
      pinMode(CS, OUTPUT);    // Ensure CS pin is not floating
      digitalWrite(CS, HIGH); // Disable chip select
      return settings;
    }

    // Open file for reading
    File file = SD.open(filename);
    if (!file) {
      Serial.println("Failed to open settings file for reading");
      SD.end();
      spi.end();
      pinMode(CS, OUTPUT);    // Ensure CS pin is not floating
      digitalWrite(CS, HIGH); // Disable chip select
      return settings;
    }

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<1024> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println(F("Failed to read file, keeping existing configuration"));
        file.close();
        SD.end();               // Deinitialize SD card
        spi.end();              // Deinitialize SPI bus
        pinMode(CS, OUTPUT);    // Ensure CS pin is not floating
        digitalWrite(CS, HIGH); // Disable chip select
        return settings;
    }

    // Copy values from the JsonDocument to the Config.
    // For missing keys, keep the existing values in `settings`.
    settings.ClockMode = doc["ClockMode"] | settings.ClockMode;
    settings.KnopenMin = doc["KnopenMin"] | settings.KnopenMin;

    strlcpy(settings.WifiSSID,
            doc["WifiSSID"] | settings.WifiSSID,
            sizeof(settings.WifiSSID));
    strlcpy(settings.WifiPWD,
            doc["WifiPWD"] | settings.WifiPWD,
            sizeof(settings.WifiPWD));
    strlcpy(settings.BlynkKey,
            doc["BlynkKey"] | settings.BlynkKey,
            sizeof(settings.BlynkKey));
    strlcpy(settings.urlActueleWind,
            doc["urlActueleWind"] | settings.urlActueleWind,
            sizeof(settings.urlActueleWind));
    strlcpy(settings.url,
            doc["url"] | settings.url,
            sizeof(settings.url));

    // MQTT / Home Assistant (optional)
    strlcpy(settings.MqttHost,
            doc["MqttHost"] | settings.MqttHost,
            sizeof(settings.MqttHost));
    settings.MqttPort = doc["MqttPort"] | settings.MqttPort;
    strlcpy(settings.MqttUser,
            doc["MqttUser"] | settings.MqttUser,
            sizeof(settings.MqttUser));
    strlcpy(settings.MqttPassword,
            doc["MqttPassword"] | settings.MqttPassword,
            sizeof(settings.MqttPassword));
    strlcpy(settings.MqttBaseTopic,
            doc["MqttBaseTopic"] | settings.MqttBaseTopic,
            sizeof(settings.MqttBaseTopic));

    // LED control
    settings.LedsOn = doc["LedsOn"] | settings.LedsOn;
    settings.Brightness = doc["Brightness"] | settings.Brightness;

    // Close the file (Curiously, File's destructor doesn't close the file)
    file.close();
    SD.end();               // Deinitialize SD card
    spi.end();              // Deinitialize SPI bus
    pinMode(CS, OUTPUT);    // Ensure CS pin is not floating
    digitalWrite(CS, HIGH); // Disable chip select
    return settings;
}


