#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>

#include <NSNWindClockLeds.h> // Staat in de Include folder
#include <NSNSDCard.h>

#define DATA_PIN 4
#define FRAMES_PER_SECOND  120

const char *filename = "/ClockSettings.json";  // <- SD library uses 8.3 filenames
ClockSettings settings;                        // <- global configuration object

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

void setup() {
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(2000);
    Serial.begin(115200);
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
    settings = LoadClockSettings(filename, settings);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(settings.WifiSSID);
    Serial.println(settings.WifiPWD); 
    
    WiFi.begin(settings.WifiSSID, settings.WifiPWD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  /*
  Blynk.begin(settings.BlynkKey, settings.WifiSSID, settings.WifiPWD);
  Blynk.setProperty(V0, "onLabel", "Clock Mode");
  Blynk.setProperty(V0, "offLabel", "Clock Mode");
  // Set Kitespots
  Blynk.setProperty(V2, "labels", "IJmuiden", "Schellinkhout");
  */
  FetchWindData();

  // Setup Interrupt Timer
  timer = timerBegin(0, 80, true); // timer 0, MWDT clock period = 12,5ns *TIMGn_Tx_WDT_CLK_PRESCALE -> 12,5 ns * 80 -> 1000 ns
  timerAttachInterrupt(timer, &onTimerSeconde, true); // edge (not level) triggered
  timerAlarmWrite(timer, 1000000, true); // 1000000 * 1 us = 1s, autoreload true
  timerAlarmEnable(timer); //enable

}

void loop() {
  
/*
  delay(500);
  LightHoog();
  LightStatus();
  LightRichting();
  LightKnopen();
  
  delay(1000);

  if(blnBlynkUpdate){
    blnBlynkUpdate = false;
    Blynk.run();
  }
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

        DynamicJsonDocument doc(payload.length()*2);
        deserializeJson(doc, payload);

        strlcpy(spot.Naam, doc["StationsNaam"] | "example.com", sizeof(spot.Naam)); 
        strlcpy(spot.Code, doc["StationsCode"] | "Dummy", sizeof(spot.Code));
        strlcpy(spot.TijdStip, doc["TijdStip"] | "Dummy", sizeof(spot.TijdStip));
         
        spot.RichtingVan = atoi(doc["VaarrichtingVan"]) | 0;
        spot.RichtingTot = atoi(doc["VaarrichtingTot"]) | 0;
        spot.Temperatuur = atof(doc["TemperatuurGC"]) * 10 ;
        spot.Wind = ConvertMSToKnots(atof(doc["WindsnelheidMS"])) | 0;
        spot.WindStoot = ConvertMSToKnots(atof(doc["WindstotenMS"])) | 0;
        spot.WindRichting = atoi(doc["WindrichtingGR"]);
        
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