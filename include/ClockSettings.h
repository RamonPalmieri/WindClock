#ifndef CLOCK_SETTINGS_H
#define CLOCK_SETTINGS_H

#include <Arduino.h>

struct ClockSettings {
    char WifiSSID[64];
    char WifiPWD[64];
    char BlynkKey[32];
    int ClockMode;
    char urlActueleWind[64];
    char url[128];
    int KnopenMin;

    // MQTT / Home Assistant
    char MqttHost[64];
    int MqttPort;
    char MqttUser[64];
    char MqttPassword[64];
    char MqttBaseTopic[64];

    // LED control
    bool LedsOn;
    int Brightness; // 0-255
};

extern ClockSettings settings;

void loadSettingsFromNVS();
void saveSettingsToNVS();

#endif