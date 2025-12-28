#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>

#include "ClockSettings.h"
#include "WindClockMqtt.h"
#include <NSNWindClockLeds.h> // Staat in de Include folder
#include <NSNSDCard.h>
#include "WindClockAP.cpp"


#define DATA_PIN 4
#define FRAMES_PER_SECOND  120

const char *filename = "/ClockSettings.json";  // <- SD library uses 8.3 filenames
//ClockSettings settings;                        // <- global configuration object

char urlActueleWind[] = "";
char url[] = "";

hw_timer_t * timer = NULL;


#define BLYNK_PRINT Serial

int ClockMode = 0;
int ClockModeMax = 4;

bool blnSuperDebug = false;
bool blnDebug = true;
bool blnClockTimeUpdate = true;
bool blnClockKiteUpdate = false;
bool blnClockDirectionUpdate = false;
bool blnClockTempUpdate = false;
bool blnFetchWindData = true;
bool blnRefreshHoogLaag = true;
bool blnHoog = false;
bool blnBlynkUpdate = false;
bool blnWindDataUpToDate = false;

String SpotCode = "6225";
String SpotNaam = "";
int SpotWind = 0;
int SpotWindStoot = 0;
int SpotWindRichting = 0;
int SpotTemperatuur = 0;
int SpotRichtingTot = 0;
int SpotRichtingVan = 0;

const int conFetchWindData = 30;
const int conChangeLaagHoog = 10; // Change clock from normal wind to high wind
int intWindUpdateTime =0;
int intHoogLaagTime = 0;

int RetryCounter = 0;      
int MaxRetryCounter = 100;
String payload = "";

#ifndef WINDCLOCK_VERSION
#define WINDCLOCK_VERSION "dev"
#endif

constexpr const char *FW_VERSION = WINDCLOCK_VERSION;

void applyLedSettings();
void requestFetchWindNow();

static bool clockSettingsEqual(const ClockSettings &a, const ClockSettings &b) {
    return a.ClockMode == b.ClockMode &&
           a.KnopenMin == b.KnopenMin &&
           a.MqttPort == b.MqttPort &&
           a.LedsOn == b.LedsOn &&
           a.Brightness == b.Brightness &&
           strncmp(a.WifiSSID, b.WifiSSID, sizeof(a.WifiSSID)) == 0 &&
           strncmp(a.WifiPWD, b.WifiPWD, sizeof(a.WifiPWD)) == 0 &&
           strncmp(a.BlynkKey, b.BlynkKey, sizeof(a.BlynkKey)) == 0 &&
           strncmp(a.urlActueleWind, b.urlActueleWind, sizeof(a.urlActueleWind)) == 0 &&
           strncmp(a.url, b.url, sizeof(a.url)) == 0 &&
           strncmp(a.MqttHost, b.MqttHost, sizeof(a.MqttHost)) == 0 &&
           strncmp(a.MqttUser, b.MqttUser, sizeof(a.MqttUser)) == 0 &&
           strncmp(a.MqttPassword, b.MqttPassword, sizeof(a.MqttPassword)) == 0 &&
           strncmp(a.MqttBaseTopic, b.MqttBaseTopic, sizeof(a.MqttBaseTopic)) == 0;
}

void IRAM_ATTR onTimerSeconde();
void ProcessWind(int);
void FetchWindData();
void ProcessError();
void ProcessDirection();
void ProcessKnopen();
void ProcessHoogLaag();
void RefreshHoogLaag();
int ConvertMSToKnots(float Speed);
CRGB GetDirectionColor(int richting);


struct WindData {
  char Naam[64];
  char Code[64];
  int RichtingVan;
  int RichtingTot;
  int Temperatuur;
  float Wind;
  float WindStoot;
  int WindRichting;
  char TijdStip[64];
  bool Error;
};

WindData spot;

void applyLedSettings() {
    int brightness = settings.Brightness;
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;

    FastLED.setBrightness((uint8_t)brightness);

    if (!settings.LedsOn) {
        LightOff();
        FastLED.show();
    }
}

void requestFetchWindNow() {
    blnFetchWindData = true;
}

void setup() {
    delay(2000);  // Allow time for power stabilization
    Serial.begin(115200);

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

    loadSettingsFromNVS();

    // If an SD card is present and ClockSettings.json contains different values
    // than NVS, persist those SD values into NVS.
    ClockSettings nvsSettings = settings;
    ClockSettings sdSettings = settings;
    sdSettings = LoadClockSettings(filename, sdSettings);
    if (!clockSettingsEqual(nvsSettings, sdSettings)) {
        Serial.println("Settings on SD differ from NVS; saving to NVS...");
        settings = sdSettings;
        saveSettingsToNVS();
    }

    bool wifiConnected = false;

    if (strlen(settings.WifiSSID) > 0 && strlen(settings.WifiPWD) > 0) {
        Serial.println("Trying WiFi...");
        Serial.print("SSID: ");
        Serial.println(settings.WifiSSID);

        WiFi.begin(settings.WifiSSID, settings.WifiPWD);
        for (int i = 0; i < 10; ++i) {
            if (WiFi.status() == WL_CONNECTED) {
                wifiConnected = true;
                break;
            }
            delay(500);
            Serial.print(".");
        }
    }

    if (wifiConnected) {
        Serial.println("\nWiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed. Starting Access Point...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ap_WindClock", "windClock");
        Serial.println("Access Point started: ap_WindClock");
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
        startAccessPointWebServer();
    }

    applyLedSettings();
    windClockMqttBegin(FW_VERSION, applyLedSettings, requestFetchWindNow);

    blnFetchWindData = true;

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimerSeconde, true);
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer);
}

void loop() {
  
/*
  delay(500);
  LightHoog();
  LightStatus();
  LightRichting();
  LightKnopen();

  */

  if(blnFetchWindData)
  {
    blnFetchWindData = false;
    FetchWindData();
  }

  if(blnRefreshHoogLaag)
  {
    blnRefreshHoogLaag = false;
    if(blnHoog)
    {
      blnHoog = false;
    }
    else
    {
      blnHoog = true;
    }
    RefreshHoogLaag();
   }

   WindClockTelemetry telemetry;
   telemetry.wind = (int)(spot.Wind + 0.5f);
   telemetry.gust = (int)(spot.WindStoot + 0.5f);
   telemetry.windDirection = spot.WindRichting;
   telemetry.directionFrom = spot.RichtingVan;
   telemetry.directionTo = spot.RichtingTot;
   telemetry.temperature = spot.Temperatuur;
   telemetry.error = spot.Error;

   windClockMqttLoop(telemetry, blnHoog);

   handleAccessPointClient();
 }

void IRAM_ATTR onTimerSeconde(){

  blnBlynkUpdate = true;

  // We use only one timer, count the seconds that we will refresh the winddata
  intWindUpdateTime += 1;
  intHoogLaagTime += 1;

  if(intWindUpdateTime > conFetchWindData)
  {
    Serial.println("Detected that we have to fetch data from the website");
    intWindUpdateTime = 0;
    blnFetchWindData = true;
  }
  if(intHoogLaagTime > conChangeLaagHoog)
  {
    Serial.println("We must change between high and low");
      intHoogLaagTime = 0;
      blnRefreshHoogLaag = true;
  }
}

void FetchWindData()
{
    /*
  // Fetch Website Data
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  IPAddress DNS =  WiFi.dnsIP();
  Serial.println(DNS);
  
  bool success = Ping.ping("actuelewind.nl", 3);
  if(!success){
      Serial.println("Ping failed");
      return;
  }
  Serial.println("Ping succesful.");

  Serial.print("FetchWindData activated");
  */
  Serial.println("We are going to fetch winddata");
  spot.Error = true;

  HTTPClient http;
  http.begin(settings.url); //Specify the URL
  int httpCode = http.GET();                                        //Make the request
 
  if (httpCode == HTTP_CODE_OK) { //Check for the returning code

      String payload = http.getString();
      Serial.print(" -> PayLoad Size: ");
      Serial.print(payload.length());

      if(payload.length() == 0)
      {
        RetryCounter += 1;
        Serial.print(" -> No Luck -> Retrying: ");
        Serial.println(RetryCounter);
        
        if(RetryCounter >= MaxRetryCounter)
        {
          //Serial.println("Stop Retry");
          RetryCounter = 0;
          blnFetchWindData = false;
          blnWindDataUpToDate = false;
          blnDebug = true;
        }
      }
      else
      {
        Serial.println(" -> YES!!");

        DynamicJsonDocument doc(payload.length() * 2);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }

        JsonObject body = doc["body"];
        if (!body) {
          Serial.println("Missing 'body' object in JSON response.");
          return;
        }

        strlcpy(spot.Naam, body["locatie"] | "example.com", sizeof(spot.Naam));
        strlcpy(spot.Code, "6225", sizeof(spot.Code));
        //spot.Code = '6225'; // We use a fixed code for now, because we only have one spot
        Serial.print("SpotCode: ");
        Serial.println(spot.Code);
        strlcpy(spot.TijdStip, "", sizeof(spot.TijdStip));

        spot.RichtingVan = atoi(body["windrichtingVan"] | "0");
        spot.RichtingTot = atoi(body["windrichtingTot"] | "0");
        spot.Temperatuur = 0;
        float windRaw = body["Windsnelheid"] | 0.0;
        float gustRaw = body["Windstoten"] | 0.0;

        spot.Wind = (int)(windRaw + 0.5);
        spot.WindStoot = (int)(gustRaw + 0.5);
        spot.WindRichting = atoi(body["windrichtingGR"] | "0");

        spot.Error = false;

        RetryCounter = 0;
        blnFetchWindData = false;
        blnWindDataUpToDate = true;
      }
  }
  else 
  {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources
  Serial.println("Done Fetching WindData");

}

void ProcessWind(int WindKnots){

  switch (WindKnots)
  {
    case 1: 
    
      LightEen();
      break;
    case 2:
      LightTwee();
      break;
    case 3:
      LightDrie();
      break;
    case 4:
      LightVier();
      break;
    case 5:
      LightVijf();
      break;
    case 6:
      LightZes();
      break;
    case 7:
      LightZeven();
      break;
    case 8:
      LightAcht();
      break;
    case 9:
      LightNegen();
      break;
    case 10:
      LightTien();
      break;
    case 11:
      LightElf();
      break;
    case 12:
      LightTwaalf();
      break;
    case 13:
      LightDertien();
      break;
    case 14:
      LightVeertien();
      break;
    case 15:
      LightVijf();
      LightTien();
      break;
    case 16:
      LightZes();
      LightTien();
      break;
    case 17:
      LightZeven();
      LightTien();
      break;
    case 18:
      LightAcht();
      LightTien();
      break;
    case 19:
      LightNegen();
      LightTien();
      break;
    case 20:
        LightTwintig();
      break;
    case 21:
      LightEen();
      LightEn();
      LightTwintig();
      break;
    case 22:
      LightTwee();
      LightEn();
      LightTwintig();
      break;
    case 23:
      LightDrie();
      LightEn();
      LightTwintig();
      break;
    case 24:
      LightVier();
      LightEn();
      LightTwintig();
      break;
    case 25:
      LightVijf();
      LightEn();
      LightTwintig();
      break;
    case 26:
      LightZes();
      LightEn();
      LightTwintig();
      break;
    case 27:
      LightZeven();
      LightEn();
      LightTwintig();
      break;
    case 28:
      LightAcht();
      LightEn();
      LightTwintig();
      break;
    case 29:
      LightNegen();
      LightEn();
      LightTwintig();
      break;

    case 30:
      LightDertig();
      break;
    case 31:
      LightEen();
      LightEn();
      LightDertig();
      break;
    case 32:
      LightTwee();
      LightEn();
      LightDertig();
      break;
    case 33:
      LightDrie();
      LightEn();
      LightDertig();
      break;
    case 34:
      LightVier();
      LightEn();
      LightDertig();
      break;
    case 35:
      LightVijf();
      LightEn();
      LightDertig();
      break;
    case 36:
      LightZes();
      LightEn();
      LightDertig();
      break;
    case 37:
      LightZeven();
      LightEn();
      LightDertig();
      break;
    case 38:
      LightAcht();
      LightEn();
      LightDertig();
      break;
    case 39:
      LightNegen();
      LightEn();
      LightDertig();
      break;

    case 40:
      LightVeertig();
      break;
    case 41:
      LightEen();
      LightEn();
      LightVeertig();
      break;
    case 42:
      LightTwee();
      LightEn();
      LightVeertig();
      break;
    case 43:
      LightDrie();
      LightEn();
      LightVeertig();
      break;
    case 44:
      LightVier();
      LightEn();
      LightVeertig();
      break;
    case 45:
      LightVijf();
      LightEn();
      LightVeertig();
      break;
    case 46:
      LightZes();
      LightEn();
      LightVeertig();
      break;
    case 47:
      LightZeven();
      LightEn();
      LightVeertig();
      break;
    case 48:
      LightAcht();
      LightEn();
      LightVeertig();
      break;
    case 49:
      LightNegen();
      LightEn();
      LightVeertig();
      break;

    case 50:
      LightVijftig();
      break;
    case 51:
      LightEen();
      LightEn();
      LightVijftig();
      break;
    case 52:
      LightTwee();
      LightEn();
      LightVijftig();
      break;
    case 53:
      LightDrie();
      LightEn();
      LightVijftig();
      break;
    case 54:
      LightVier();
      LightEn();
      LightVijftig();
      break;
    case 55:
      LightVijf();
      LightEn();
      LightVijftig();
      break;
    case 56:
      LightZes();
      LightEn();
      LightVijftig();
      break;
    case 57:
      LightZeven();
      LightEn();
      LightVijftig();
      break;
    case 58:
      LightAcht();
      LightEn();
      LightVijftig();
      break;
    case 59:
      LightNegen();
      LightEn();
      LightVijftig();
      break;
    default:
      // LightInsane();
      break;
  }

    FastLED.show();
    FastLED.delay(1000/FRAMES_PER_SECOND);
}

void RefreshHoogLaag()
{
    if (!settings.LedsOn) {
        LightOff();
        FastLED.show();
        return;
    }

    LightOff();
    ProcessError();
    if(blnHoog)
    {
      ProcessWind(spot.WindStoot);
      LightHoog(CRGB::Red);
    }
    else
    {
      ProcessWind(spot.Wind);
      LightHoog(CRGB::Black);
    }
    
    ProcessDirection();
    ProcessKnopen();
    
    FastLED.show();
    FastLED.delay(1000/FRAMES_PER_SECOND);
    Serial.println("Done refreshing clock");
}

void ProcessError()
{
  if(spot.Error)
  {
    LightStatus(CRGB::Red);
  }
  else
  {
    LightStatus(CRGB::Black);
  }
}

void ProcessDirection()
{
  CRGB color = GetDirectionColor(spot.WindRichting);
  LightRichting(color);
}

void ProcessKnopen()
{
  Serial.println(spot.Wind);
  if(spot.Wind >= settings.KnopenMin)
  {
    LightKnopen(CRGB::Green);
  }
  else
  {
    LightKnopen(CRGB::Red);
  }
  
}

int ConvertMSToKnots(float Speed){
  
  float Knots = Speed / 0.5144;
  Knots += 0.5;
  int KnotsRounded = (int)Knots;
  
  return KnotsRounded;
}

CRGB GetDirectionColor(int richting){

  // 185-344 graden Groen -> DirectionColorGood
  // xxx-xxx graden Geel -> DirectionColorOK # aan de kust is het 180 graden gesplitst. of goed of niks
  // 345-184 graden Rood -> DirectionColorBad

  CRGB DirectionColorGood = CRGB::Green;
  CRGB DirectionColorBad = CRGB::Red;

  if(spot.RichtingVan == spot.RichtingTot){
    // WindrichtingData onbekend
      if(blnDebug){Serial.println("Richting is goed.");}
      return DirectionColorGood;
  }
  if(spot.RichtingVan > spot.RichtingTot)
  {
    // we gaan over de 360 graden heen
    if(spot.WindRichting >= spot.RichtingVan)
    {
      // Goede richting
      if(blnDebug){Serial.println("Richting is goed..");}
      return DirectionColorGood;
    }
    if(spot.WindRichting <= spot.RichtingTot)
    {
      // Goede richting
      if(blnDebug){Serial.println("Richting is goed...");}
    }
    if(blnDebug){Serial.println("Richting is slecht");}
    return DirectionColorBad;
  }


  if(SpotWindRichting >= SpotRichtingVan)
  {
    if(SpotWindRichting <= SpotRichtingTot)
    {
      if(blnDebug){Serial.println("Richting is goed");}
      return DirectionColorGood;
    }
    if(blnDebug){Serial.println("Richting is slecht");}
    return DirectionColorBad;
  }
  if(blnDebug){Serial.println("Richting is slecht");}
  return DirectionColorBad;
}