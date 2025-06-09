
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
      return settings;
    }

    // Open file for reading
    File file = SD.open(filename);

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));

    // Copy values from the JsonDocument to the Config
    settings.ClockMode = doc["ClockMode"] | 0;
    settings.KnopenMin = doc["KnopenMin"] | 0;
    strlcpy(settings.WifiSSID,                  // <- destination
            doc["WifiSSID"] | "example.com",  // <- source
            sizeof(settings.WifiSSID));         // <- destination's capacity
    strlcpy(settings.WifiPWD,                  // <- destination
            doc["WifiPWD"] | "fakepassword",  // <- source
            sizeof(settings.WifiPWD));         // <- destination's capacity
    strlcpy(settings.BlynkKey,                  
            doc["BlynkKey"] | "fakekey",  
            sizeof(settings.BlynkKey)); 
    strlcpy(settings.urlActueleWind,                  
            doc["urlActueleWind"] | "default",  
            sizeof(settings.urlActueleWind)); 
    strlcpy(settings.url,                  
            doc["url"] | "default",  
            sizeof(settings.url)); 

    // Close the file (Curiously, File's destructor doesn't close the file)
    file.close();
    SD.end();               // Deinitialize SD card
    spi.end();              // Deinitialize SPI bus
    pinMode(CS, OUTPUT);    // Ensure CS pin is not floating
    digitalWrite(CS, HIGH); // Disable chip select
    return settings;
}


