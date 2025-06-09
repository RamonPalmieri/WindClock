#include "ClockSettings.h"
#include <Preferences.h>

Preferences prefs;
ClockSettings settings;

void loadSettingsFromNVS() {
    prefs.begin("windclock", true);  // read-only
    prefs.getString("ssid", "").toCharArray(settings.WifiSSID, sizeof(settings.WifiSSID));
    prefs.getString("pwd", "").toCharArray(settings.WifiPWD, sizeof(settings.WifiPWD));
    prefs.getString("blynk", "").toCharArray(settings.BlynkKey, sizeof(settings.BlynkKey));
    settings.ClockMode = prefs.getInt("mode", 0);
    prefs.getString("urlAW", "").toCharArray(settings.urlActueleWind, sizeof(settings.urlActueleWind));
    prefs.getString("url", "").toCharArray(settings.url, sizeof(settings.url));
    settings.KnopenMin = prefs.getInt("knopen", 0);
    prefs.end();
}

void saveSettingsToNVS() {
    prefs.begin("windclock", false);  // writeable
    prefs.putString("ssid", String(settings.WifiSSID));
    prefs.putString("pwd", String(settings.WifiPWD));
    prefs.putString("blynk", String(settings.BlynkKey));
    prefs.putInt("mode", settings.ClockMode);
    prefs.putString("urlAW", String(settings.urlActueleWind));
    prefs.putString("url", String(settings.url));
    prefs.putInt("knopen", settings.KnopenMin);
    prefs.end();
}