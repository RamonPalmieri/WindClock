#include "ClockSettings.h"
#include <Preferences.h>

Preferences prefs;
ClockSettings settings;

void loadSettingsFromNVS() {
    prefs.begin("windclock", true);  // read-only


Serial.println("Loading settings from NVS...");

    prefs.getString("ssid", "").toCharArray(settings.WifiSSID, sizeof(settings.WifiSSID));
    prefs.getString("pwd", "").toCharArray(settings.WifiPWD, sizeof(settings.WifiPWD));
    prefs.getString("blynk", "").toCharArray(settings.BlynkKey, sizeof(settings.BlynkKey));
    settings.ClockMode = prefs.getInt("mode", 0);

    prefs.getString("urlAW", "").toCharArray(settings.urlActueleWind, sizeof(settings.urlActueleWind));
    prefs.getString("url", "").toCharArray(settings.url, sizeof(settings.url));
    settings.KnopenMin = prefs.getInt("knopen", 0);

    // MQTT defaults: disabled unless host is set
    prefs.getString("mqhost", "").toCharArray(settings.MqttHost, sizeof(settings.MqttHost));
    settings.MqttPort = prefs.getInt("mqport", 1883);
    prefs.getString("mquser", "").toCharArray(settings.MqttUser, sizeof(settings.MqttUser));
    prefs.getString("mqpass", "").toCharArray(settings.MqttPassword, sizeof(settings.MqttPassword));
    // Device/base topic (NOT the Home Assistant discovery prefix).
    // The HA discovery prefix is usually "homeassistant" and is handled in WindClockMqtt.
    prefs.getString("mqtopic", "windclock").toCharArray(settings.MqttBaseTopic, sizeof(settings.MqttBaseTopic));

    settings.LedsOn = prefs.getBool("ledson", true);
    settings.Brightness = prefs.getInt("bright", 128);

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

    prefs.putString("mqhost", String(settings.MqttHost));
    prefs.putInt("mqport", settings.MqttPort);
    prefs.putString("mquser", String(settings.MqttUser));
    prefs.putString("mqpass", String(settings.MqttPassword));
    prefs.putString("mqtopic", String(settings.MqttBaseTopic));

    prefs.putBool("ledson", settings.LedsOn);
    prefs.putInt("bright", settings.Brightness);

    prefs.end();
}