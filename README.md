# WindClock

WindClock is an ESP32 (Arduino/PlatformIO) firmware project for a laser-cut “wind clock”: a WS2812B LED word/segment display that shows live wind conditions (wind speed, gusts, and direction) fetched from a configurable HTTP endpoint.

## What the firmware does

- Connects to WiFi using saved credentials.
- Periodically fetches wind data over HTTP (`settings.url`) and parses a JSON response.
- Drives a WS2812B LED chain via FastLED (data pin `GPIO4`).
- Publishes telemetry to MQTT (optional) and supports Home Assistant MQTT Discovery.
- Supports over-the-air firmware updates (ArduinoOTA).
- Persists configuration in ESP32 NVS (Preferences), with optional loading from an SD card JSON file.

## Runtime behavior (high level)

- Every ~30 seconds the device fetches wind data from the configured URL.
- The display alternates between:
  - normal wind speed, and
  - “high” mode (gusts)
  roughly every ~10 seconds.
- The status indicator turns red when the last fetch resulted in an error.
- A “knopen” (knots) indicator changes color based on whether the current wind speed is above the configured minimum threshold (`KnopenMin`).
- Wind direction is shown using a “good/bad” color depending on whether the measured direction is inside the allowed range returned by the data source.

## Configuration and persistence

The `ClockSettings` struct holds all configurable values (WiFi, wind-data URL, MQTT, and LED settings).

- **Primary storage:** ESP32 NVS (Preferences).
- **Optional SD card import:** On boot, the firmware attempts to read `/ClockSettings.json` from an SD card and, if it differs from NVS, saves the SD values into NVS.

### Access Point fallback setup

If WiFi connection fails, the ESP32 starts an Access Point:

- SSID: `ap_WindClock`
- Password: `windClock`

It serves a small setup web UI where you can configure:

- WiFi SSID/password
- Wind data URL (and optional station code)
- `KnopenMin` threshold
- MQTT broker settings (optional)
- LED on/off + brightness

## OTA updates

When connected to WiFi, the firmware enables ArduinoOTA.

- Hostname pattern: `windclock-<last6hex>` (derived from the ESP32 MAC)
- There is an OTA PlatformIO environment in `platformio.ini` (`esp32-wroom-32d_ota`).
- The OTA password is expected via environment variable `OTA_PASSWORD` (so secrets don’t live in git).

## MQTT / Home Assistant integration (optional)

If `MqttHost` is set (non-empty), the firmware connects to the broker and:

- Publishes availability (`online`/`offline`) and a retained JSON state payload containing wind telemetry and current settings.
- Publishes Home Assistant MQTT Discovery messages so the device appears automatically (sensors + a light entity for on/off + brightness).
- Subscribes to `.../set/#` topics to allow changing settings like station URL, station code, `knopen_min`, and light state/brightness; changes are persisted to NVS.

## Build / flash

This is a PlatformIO project.

- USB upload: `pio run -e esp32-wroom-32d -t upload`
- Serial monitor: `pio device monitor -e esp32-wroom-32d`
- OTA upload: `pio run -e esp32-wroom-32d_ota -t upload --upload-port <ip-or-mdns>`
