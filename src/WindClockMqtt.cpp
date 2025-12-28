#include "WindClockMqtt.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "ClockSettings.h"

namespace {
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void (*applyLedSettingsFn)() = nullptr;
void (*requestFetchNowFn)() = nullptr;

String deviceId;
String baseTopic;
String deviceTopic;
String availabilityTopic;
String fwVersionStored = "dev";

bool discoveryPublished = false;
unsigned long lastReconnectAttemptMs = 0;
unsigned long lastStatePublishMs = 0;
unsigned long lastLightPublishMs = 0;

WindClockTelemetry lastTelemetry{};
bool lastHighMode = false;
bool hasLastTelemetry = false;

String makeDeviceId() {
    uint64_t mac = ESP.getEfuseMac();
    char buf[32];
    snprintf(buf, sizeof(buf), "windclock_%04X%08X", (uint16_t)(mac >> 32), (uint32_t)mac);
    return String(buf);
}

bool mqttConfigured() {
    return strlen(settings.MqttHost) > 0;
}

void publishJson(const String &topic, const JsonDocument &doc, bool retained) {
    String payload;
    serializeJson(doc, payload);
    mqttClient.publish(topic.c_str(), payload.c_str(), retained);
}

void publishAvailability(bool online) {
    mqttClient.publish(availabilityTopic.c_str(), online ? "online" : "offline", true);
}

void publishLightState() {
    if (!mqttClient.connected()) return;

    StaticJsonDocument<96> doc;
    doc["state"] = settings.LedsOn ? "ON" : "OFF";
    int brightness = settings.Brightness;
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    doc["brightness"] = brightness;

    publishJson(deviceTopic + "/state/light", doc, true);
}

String extractStationFromUrl(const char *url) {
    if (!url || url[0] == '\0') return "";

    String s(url);
    int idx = s.indexOf("station=");
    if (idx < 0) return "";

    idx += 8;
    int end = idx;
    while (end < (int)s.length() && isDigit(s[end])) {
        end++;
    }
    return s.substring(idx, end);
}

String replaceOrAppendStationParam(const char *url, const String &station) {
    if (!url || url[0] == '\0') return "";

    String s(url);
    int idx = s.indexOf("station=");
    if (idx >= 0) {
        int valueStart = idx + 8;
        int valueEnd = valueStart;
        while (valueEnd < (int)s.length() && isDigit(s[valueEnd])) {
            valueEnd++;
        }
        return s.substring(0, valueStart) + station + s.substring(valueEnd);
    }

    if (s.indexOf('?') >= 0) {
        return s + "&station=" + station;
    }
    return s + "?station=" + station;
}

void publishState(const WindClockTelemetry &telemetry, bool highMode, bool retained) {
    if (!mqttClient.connected()) return;

    StaticJsonDocument<384> doc;
    doc["wind"] = telemetry.wind;
    doc["gust"] = telemetry.gust;
    doc["wind_direction"] = telemetry.windDirection;
    doc["direction_from"] = telemetry.directionFrom;
    doc["direction_to"] = telemetry.directionTo;
    doc["temperature"] = telemetry.temperature;
    doc["error"] = telemetry.error;
    doc["high_mode"] = highMode;

    doc["knopen_min"] = settings.KnopenMin;
    doc["url"] = settings.url;
    doc["station"] = extractStationFromUrl(settings.url);

    doc["leds_on"] = settings.LedsOn;
    doc["brightness"] = settings.Brightness;

    doc["ip"] = WiFi.isConnected() ? WiFi.localIP().toString() : "";

    publishJson(deviceTopic + "/state", doc, retained);
}

void publishDiscovery(const char *fwVersion) {
   
    if (!mqttClient.connected()) return;
    Serial.println("MQTT connected.");

    const String stateTopic = deviceTopic + "/state";
    const String lightStateTopic = deviceTopic + "/state/light";

    const String urlSetTopic = deviceTopic + "/set/url";
    const String stationSetTopic = deviceTopic + "/set/station";
    const String knopenMinSetTopic = deviceTopic + "/set/knopen_min";
    const String lightSetTopic = deviceTopic + "/set/light";

    auto deviceObj = [&](JsonObject obj) {
        JsonArray ids = obj.createNestedArray("identifiers");
        ids.add(deviceId);
        obj["name"] = "WindClock";
        obj["manufacturer"] = "Non Stop Networking";
        obj["model"] = "ESP32 Kite WindClock";
        obj["sw_version"] = fwVersion;
    };

    // --- Light (on/off + brightness)
    {
        StaticJsonDocument<512> doc;
        doc["name"] = "WindClock";
        doc["unique_id"] = deviceId + "_light";
        doc["schema"] = "json";
        doc["command_topic"] = lightSetTopic;
        doc["state_topic"] = lightStateTopic;
        doc["availability_topic"] = availabilityTopic;
        doc["payload_available"] = "online";
        doc["payload_not_available"] = "offline";
        doc["brightness"] = true;
        JsonArray colorModes = doc.createNestedArray("supported_color_modes");
        colorModes.add("brightness");
        JsonObject dev = doc.createNestedObject("device");
        deviceObj(dev);
        
        publishJson("homeassistant/light/" + deviceId + "/windclock/config", doc, true);
    }

    // --- Sensors
    struct SensorDef {
        const char *component;
        const char *objectId;
        const char *name;
        const char *valueTemplate;
        const char *unit;
        const char *deviceClass;
    };

    const SensorDef sensors[] = {
        {"sensor", "wind", "Wind Speed", "{{ value_json.wind }}", "kn", "wind_speed"},
        {"sensor", "gust", "Wind Gust", "{{ value_json.gust }}", "kn", "wind_speed"},
        {"sensor", "wind_direction", "Wind Direction", "{{ value_json.wind_direction }}", "°", nullptr},
        {"sensor", "station", "Station", "{{ value_json.station }}", nullptr, nullptr},
        {"sensor", "ip", "IP", "{{ value_json.ip }}", nullptr, nullptr},
    };

    for (const auto &s : sensors) {
        StaticJsonDocument<512> doc;
        doc["name"] = s.name;
        doc["unique_id"] = deviceId + "_" + s.objectId;
        doc["state_topic"] = stateTopic;
        doc["value_template"] = s.valueTemplate;
        if (s.unit) doc["unit_of_measurement"] = s.unit;
        if (s.deviceClass) doc["device_class"] = s.deviceClass;
        doc["availability_topic"] = availabilityTopic;
        doc["payload_available"] = "online";
        doc["payload_not_available"] = "offline";
        JsonObject dev = doc.createNestedObject("device");
        deviceObj(dev);
        publishJson(String("homeassistant/") + s.component + "/" + deviceId + "/" + s.objectId + "/config", doc, true);
    }

    // --- Number: knopen_min
    {
        StaticJsonDocument<640> doc;
        doc["name"] = "Knopen Min";
        doc["unique_id"] = deviceId + "_knopen_min";
        doc["state_topic"] = stateTopic;
        doc["value_template"] = "{{ value_json.knopen_min }}";
        doc["command_topic"] = knopenMinSetTopic;
        doc["min"] = 0;
        doc["max"] = 60;
        doc["step"] = 1;
        doc["mode"] = "box";
        doc["availability_topic"] = availabilityTopic;
        doc["payload_available"] = "online";
        doc["payload_not_available"] = "offline";
        JsonObject dev = doc.createNestedObject("device");
        deviceObj(dev);

        publishJson("homeassistant/number/" + deviceId + "/knopen_min/config", doc, true);
    }

    // --- Text: URL
    {
        StaticJsonDocument<768> doc;
        doc["name"] = "Station URL";
        doc["unique_id"] = deviceId + "_url";
        doc["state_topic"] = stateTopic;
        doc["value_template"] = "{{ value_json.url }}";
        doc["command_topic"] = urlSetTopic;
        doc["mode"] = "text";
        doc["max"] = 128;
        doc["availability_topic"] = availabilityTopic;
        doc["payload_available"] = "online";
        doc["payload_not_available"] = "offline";
        JsonObject dev = doc.createNestedObject("device");
        deviceObj(dev);

        publishJson("homeassistant/text/" + deviceId + "/url/config", doc, true);
    }

    // --- Text: station code (helper)
    {
        StaticJsonDocument<768> doc;
        doc["name"] = "Station Code";
        doc["unique_id"] = deviceId + "_station";
        doc["state_topic"] = stateTopic;
        doc["value_template"] = "{{ value_json.station }}";
        doc["command_topic"] = stationSetTopic;
        doc["mode"] = "text";
        doc["max"] = 16;
        doc["availability_topic"] = availabilityTopic;
        doc["payload_available"] = "online";
        doc["payload_not_available"] = "offline";
        JsonObject dev = doc.createNestedObject("device");
        deviceObj(dev);

        publishJson("homeassistant/text/" + deviceId + "/station/config", doc, true);
    }

    discoveryPublished = true;
}

void handleLightSet(const String &payload) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) return;

    if (doc.containsKey("state")) {
        const char *state = doc["state"].as<const char *>();
        if (state) settings.LedsOn = (String(state) == "ON");
    }

    if (doc.containsKey("brightness")) {
        int b = doc["brightness"].as<int>();
        if (b < 0) b = 0;
        if (b > 255) b = 255;
        settings.Brightness = b;
    }

    saveSettingsToNVS();
    if (applyLedSettingsFn) applyLedSettingsFn();

    lastStatePublishMs = 0;
    publishLightState();
}

void handleUrlSet(const String &payload) {
    if (payload.length() == 0) return;

    payload.toCharArray(settings.url, sizeof(settings.url));
    saveSettingsToNVS();

    lastStatePublishMs = 0;
    if (requestFetchNowFn) requestFetchNowFn();
}

void handleStationSet(const String &payload) {
    String station = payload;
    station.trim();
    if (station.length() == 0) return;

    String newUrl = replaceOrAppendStationParam(settings.url, station);
    if (newUrl.length() == 0) return;

    newUrl.toCharArray(settings.url, sizeof(settings.url));
    saveSettingsToNVS();

    lastStatePublishMs = 0;
    if (requestFetchNowFn) requestFetchNowFn();
}

void handleKnopenMinSet(const String &payload) {
    int value = payload.toInt();
    if (value < 0) value = 0;
    if (value > 60) value = 60;

    settings.KnopenMin = value;
    saveSettingsToNVS();
    lastStatePublishMs = 0;
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    String topicStr(topic);
    String payloadStr;
    payloadStr.reserve(length + 1);
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    const String lightTopic = deviceTopic + "/set/light";
    const String urlTopic = deviceTopic + "/set/url";
    const String stationTopic = deviceTopic + "/set/station";
    const String knopenMinTopic = deviceTopic + "/set/knopen_min";

    if (topicStr == lightTopic) {
        handleLightSet(payloadStr);
        return;
    }
    if (topicStr == urlTopic) {
        handleUrlSet(payloadStr);
        return;
    }
    if (topicStr == stationTopic) {
        handleStationSet(payloadStr);
        return;
    }
    if (topicStr == knopenMinTopic) {
        handleKnopenMinSet(payloadStr);
        return;
    }
}

bool ensureConnected(const char *fwVersion) {
    if (!mqttConfigured()) return false;
    if (WiFi.status() != WL_CONNECTED) return false;
    if (mqttClient.connected()) return true;

    unsigned long now = millis();
    if (now - lastReconnectAttemptMs < 5000) return false;
    lastReconnectAttemptMs = now;

    mqttClient.setServer(settings.MqttHost, settings.MqttPort);
    mqttClient.setCallback(mqttCallback);

    String clientId = deviceId;

    const char *user = strlen(settings.MqttUser) ? settings.MqttUser : nullptr;
    const char *pass = strlen(settings.MqttPassword) ? settings.MqttPassword : nullptr;

    bool ok;
    if (user) {
        ok = mqttClient.connect(clientId.c_str(), user, pass, availabilityTopic.c_str(), 0, true, "offline");
    } else {
        ok = mqttClient.connect(clientId.c_str(), availabilityTopic.c_str(), 0, true, "offline");
    }

    if (!ok) return false;

    publishAvailability(true);

    mqttClient.subscribe((deviceTopic + "/set/#").c_str());

    publishDiscovery(fwVersion);
    publishLightState();

    return true;
}
} // namespace

void windClockMqttBegin(const char *fwVersion, void (*applyLedSettings)(), void (*requestFetchNow)()) {

    if (fwVersion && fwVersion[0] != '\0') {
        fwVersionStored = fwVersion;
    }

    applyLedSettingsFn = applyLedSettings;
    requestFetchNowFn = requestFetchNow;

    deviceId = makeDeviceId();

    // `settings.MqttBaseTopic` is the device topic root (e.g. "windclock").
    // Home Assistant's MQTT discovery prefix is typically "homeassistant" and must not
    // be used as the device base topic.
    baseTopic = strlen(settings.MqttBaseTopic) ? String(settings.MqttBaseTopic) : String("windclock");
    if (baseTopic == "homeassistant") {
        Serial.println("MqttBaseTopic is set to 'homeassistant' (discovery prefix). Using 'windclock' for device topics instead.");
        baseTopic = "windclock";
    }

    deviceTopic = baseTopic + "/" + deviceId;
    availabilityTopic = deviceTopic + "/status";
    discoveryPublished = false;
    lastReconnectAttemptMs = 0;
    lastStatePublishMs = 0;
    lastLightPublishMs = 0;
    hasLastTelemetry = false;

    if (mqttConfigured()) {
        ensureConnected(fwVersion);
    }
}

void windClockMqttLoop(const WindClockTelemetry &telemetry, bool highMode) {
    if (!mqttConfigured()) return;

    ensureConnected(fwVersionStored.c_str());

    if (!mqttClient.connected()) return;

    mqttClient.loop();
    unsigned long now = millis();

    bool telemetryChanged = !hasLastTelemetry ||
                           telemetry.wind != lastTelemetry.wind ||
                           telemetry.gust != lastTelemetry.gust ||
                           telemetry.windDirection != lastTelemetry.windDirection ||
                           telemetry.directionFrom != lastTelemetry.directionFrom ||
                           telemetry.directionTo != lastTelemetry.directionTo ||
                           telemetry.temperature != lastTelemetry.temperature ||
                           telemetry.error != lastTelemetry.error ||
                           highMode != lastHighMode;

    if (telemetryChanged || (now - lastStatePublishMs > 10000)) {
        publishState(telemetry, highMode, true);
        lastStatePublishMs = now;
        lastTelemetry = telemetry;
        lastHighMode = highMode;
        hasLastTelemetry = true;
    }

    if (now - lastLightPublishMs > 30000) {
        publishLightState();
        lastLightPublishMs = now;
    }
}

bool windClockMqttConnected() {
    return mqttClient.connected();
}
