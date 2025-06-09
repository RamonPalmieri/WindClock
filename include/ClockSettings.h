#ifndef CLOCK_SETTINGS_H
#define CLOCK_SETTINGS_H

#include <Arduino.h>

struct ClockSettings {
    char WifiSSID[64];
    char WifiPWD[64];
    char BlynkKey[32];
    int ClockMode;
    char urlActueleWind[64];
    char url[64];
    int KnopenMin;
};

extern ClockSettings settings;

void loadSettingsFromNVS();
void saveSettingsToNVS();

#endif