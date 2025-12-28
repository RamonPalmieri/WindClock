#pragma once

#include <Arduino.h>

struct WindClockTelemetry {
    int wind;
    int gust;
    int windDirection;
    int directionFrom;
    int directionTo;
    int temperature;
    bool error;
};

// Starts MQTT (if configured) and publishes HA discovery.
// - `applyLedSettings`: called after on/off/brightness changes.
// - `requestFetchNow`: called after URL/station changes.
void windClockMqttBegin(const char *fwVersion, void (*applyLedSettings)(), void (*requestFetchNow)());

// Keeps MQTT connected and publishes state periodically / on changes.
void windClockMqttLoop(const WindClockTelemetry &telemetry, bool highMode);

// True when MQTT is configured and connected.
bool windClockMqttConnected();
