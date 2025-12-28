#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "ClockSettings.h"

WebServer apServer(80);

namespace {
String htmlEscape(const String &input) {
    String out;
    out.reserve(input.length());

    for (size_t i = 0; i < input.length(); i++) {
        const char c = input[i];
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c; break;
        }
    }

    return out;
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

String replaceOrAppendStationParam(const String &url, const String &station) {
    if (url.length() == 0) return url;

    int idx = url.indexOf("station=");
    if (idx >= 0) {
        int valueStart = idx + 8;
        int valueEnd = valueStart;
        while (valueEnd < (int)url.length() && isDigit(url[valueEnd])) {
            valueEnd++;
        }
        return url.substring(0, valueStart) + station + url.substring(valueEnd);
    }

    if (url.indexOf('?') >= 0) {
        return url + "&station=" + station;
    }
    return url + "?station=" + station;
}

String buildConfigPageHtml(const String &messageHtml) {
    const String ssid = htmlEscape(String(settings.WifiSSID));
    const String url = htmlEscape(String(settings.url));
    const String station = htmlEscape(extractStationFromUrl(settings.url));

    const String mqttHost = htmlEscape(String(settings.MqttHost));
    const String mqttUser = htmlEscape(String(settings.MqttUser));
    const String mqttBase = htmlEscape(String(settings.MqttBaseTopic));

    const int mqttPort = settings.MqttPort > 0 ? settings.MqttPort : 1883;

    int brightness = settings.Brightness;
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;

    int knopenMin = settings.KnopenMin;
    if (knopenMin < 0) knopenMin = 0;
    if (knopenMin > 60) knopenMin = 60;

    const char *ledsChecked = settings.LedsOn ? "checked" : "";

    String html;
    html.reserve(5500);

    html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>WindClock Setup</title>";
    html += R"rawliteral(
<style>
:root{--bg:#0b1220;--card:#111a2e;--text:#e8eefc;--muted:#aab7dd;--accent:#4f8cff;--danger:#ff5a7a;--ok:#35d07f;--border:rgba(255,255,255,.08)}
*{box-sizing:border-box}
body{margin:0;font-family:ui-sans-serif,system-ui,-apple-system,Segoe UI,Roboto,Helvetica,Arial; background:radial-gradient(1000px 700px at 15% 0%, #172a55 0%, var(--bg) 55%); color:var(--text)}
.container{max-width:920px;margin:0 auto;padding:20px}
.header{display:flex;align-items:center;justify-content:space-between;gap:16px;margin:12px 0 18px}
.brand{display:flex;align-items:center;gap:12px}
.logo{width:42px;height:42px;border-radius:12px;background:linear-gradient(135deg,var(--accent),#7c4dff);box-shadow:0 10px 30px rgba(79,140,255,.25)}
.hgroup h1{margin:0;font-size:20px;letter-spacing:.2px}
.hgroup p{margin:2px 0 0;color:var(--muted);font-size:13px}
.card{background:rgba(17,26,46,.9);border:1px solid var(--border);border-radius:16px;padding:16px 16px 14px; box-shadow:0 18px 40px rgba(0,0,0,.35)}
.grid{display:grid;grid-template-columns:repeat(12,1fr);gap:12px}
.section-title{grid-column:1/-1;margin:0 0 2px;font-size:14px;color:var(--muted);text-transform:uppercase;letter-spacing:.12em}
.field{grid-column:span 6}
.field.wide{grid-column:span 12}
.field.third{grid-column:span 4}
.label{display:block;font-size:12px;color:var(--muted);margin:0 0 6px}
.input, .select{width:100%;padding:11px 12px;border-radius:12px;border:1px solid var(--border);background:rgba(255,255,255,.04);color:var(--text);outline:none}
.input:focus{border-color:rgba(79,140,255,.6);box-shadow:0 0 0 3px rgba(79,140,255,.18)}
.help{margin:6px 0 0;font-size:12px;color:rgba(170,183,221,.9)}
.row{display:flex;align-items:center;gap:12px}
.checkbox{display:flex;align-items:center;gap:10px;padding:10px 12px;border-radius:12px;border:1px solid var(--border);background:rgba(255,255,255,.03)}
.checkbox input{width:18px;height:18px}
.actions{display:flex;gap:10px;align-items:center;justify-content:flex-end;margin-top:14px}
.btn{appearance:none;border:1px solid var(--border);background:rgba(255,255,255,.06);color:var(--text);padding:11px 14px;border-radius:12px;font-weight:600;cursor:pointer}
.btn.primary{background:linear-gradient(135deg,var(--accent),#7c4dff);border-color:transparent}
.banner{margin:0 0 14px;padding:12px 12px;border-radius:14px;border:1px solid var(--border);background:rgba(255,255,255,.05)}
.banner.ok{border-color:rgba(53,208,127,.35);background:rgba(53,208,127,.10)}
.banner.danger{border-color:rgba(255,90,122,.35);background:rgba(255,90,122,.10)}
small.code{font-family:ui-monospace,SFMono-Regular,Menlo,Monaco,Consolas,monospace;color:var(--muted)}
@media (max-width:760px){.field{grid-column:span 12}.field.third{grid-column:span 12}.header{flex-direction:column;align-items:flex-start}}
</style>
)rawliteral";

    html += R"rawliteral(
</head><body>
<div class='container'>
  <div class='header'>
    <div class='brand'>
      <div class='logo'></div>
      <div class='hgroup'>
        <h1>WindClock Setup</h1>
        <p>Configure WiFi, MQTT and display settings</p>
      </div>
    </div>
    <div><small class='code'>AP: ap_WindClock / windClock</small></div>
  </div>
)rawliteral";

    if (messageHtml.length() > 0) {
        html += messageHtml;
    }

    html += "<div class='card'>";
    html += "<form action='/save' method='post'>";
    html += "<div class='grid'>";

    html += "<div class='section-title'>WiFi</div>";

    html += "<div class='field'>";
    html += "<label class='label' for='ssid'>SSID</label>";
    html += "<input class='input' id='ssid' name='ssid' type='text' maxlength='63' value='" + ssid + "' placeholder='Your WiFi name'>";
    html += "</div>";

    html += "<div class='field'>";
    html += "<label class='label' for='password'>Password</label>";
    html += "<input class='input' id='password' name='password' type='password' maxlength='63' value='' placeholder='WiFi password (leave empty to keep)'>";
    html += "<div class='help'>Leave empty to keep the saved password.</div>";
    html += "</div>";

    html += "<div class='section-title'>Wind Data</div>";

    html += "<div class='field wide'>";
    html += "<label class='label' for='url'>Station URL</label>";
    html += "<input class='input' id='url' name='url' type='text' maxlength='127' value='" + url + "' placeholder='http://.../windstats.php?station=6225'>";
    html += "</div>";

    html += "<div class='field'>";
    html += "<label class='label' for='station'>Station Code (optional)</label>";
    html += "<input class='input' id='station' name='station' type='text' maxlength='16' value='" + station + "' placeholder='6225'>";
    html += "<div class='help'>If set, updates/creates the <small class='code'>station=</small> query parameter in the URL.</div>";
    html += "</div>";

    html += "<div class='field'>";
    html += "<label class='label' for='knopen_min'>Knopen Min</label>";
    html += "<input class='input' id='knopen_min' name='knopen_min' type='number' min='0' max='60' step='1' value='" + String(knopenMin) + "'>";
    html += "</div>";

    html += "<div class='section-title'>MQTT (Home Assistant)</div>";

    html += "<div class='field'>";
    html += "<label class='label' for='mqtt_host'>Broker Host</label>";
    html += "<input class='input' id='mqtt_host' name='mqtt_host' type='text' maxlength='63' value='" + mqttHost + "' placeholder='192.168.1.10'>";
    html += "<div class='help'>Leave empty to disable MQTT.</div>";
    html += "</div>";

    html += "<div class='field third'>";
    html += "<label class='label' for='mqtt_port'>Port</label>";
    html += "<input class='input' id='mqtt_port' name='mqtt_port' type='number' min='1' max='65535' step='1' value='" + String(mqttPort) + "'>";
    html += "</div>";

    html += "<div class='field third'>";
    html += "<label class='label' for='mqtt_user'>Username</label>";
    html += "<input class='input' id='mqtt_user' name='mqtt_user' type='text' maxlength='63' value='" + mqttUser + "' placeholder='(optional)'>";
    html += "</div>";

    html += "<div class='field third'>";
    html += "<label class='label' for='mqtt_password'>Password</label>";
    html += "<input class='input' id='mqtt_password' name='mqtt_password' type='password' maxlength='63' value='' placeholder='(leave empty to keep)'>";
    html += "</div>";

    html += "<div class='field'>";
    html += "<label class='label' for='mqtt_base'>Base Topic</label>";
    html += "<input class='input' id='mqtt_base' name='mqtt_base' type='text' maxlength='63' value='" + mqttBase + "' placeholder='windclock'>";
    html += "</div>";

    html += "<div class='section-title'>LED Display</div>";

    html += "<div class='field'>";
    html += "<label class='label'>Power</label>";
    html += "<div class='checkbox'><input id='leds_on' name='leds_on' type='checkbox' " + String(ledsChecked) + ">";
    html += "<label for='leds_on'>LEDs On</label></div>";
    html += "</div>";

    html += "<div class='field'>";
    html += "<label class='label' for='brightness'>Brightness</label>";
    html += "<div class='row'>";
    html += "<input class='input' style='padding:10px 12px' id='brightness' name='brightness' type='range' min='0' max='255' value='" + String(brightness) + "' oninput='brightness_value.value=this.value'>";
    html += "<input class='input' style='max-width:110px' id='brightness_value' type='number' min='0' max='255' value='" + String(brightness) + "' oninput='brightness.value=this.value'>";
    html += "</div></div>";

    html += "</div>"; // grid

    html += "<div class='actions'>";
    html += "<button class='btn primary' type='submit'>Save</button>";
    html += "</div>";

    html += "</form></div>";

    html += "</div></body></html>";

    return html;
}

void sendMessageBanner(const String &type, const String &title, const String &details) {
    const String klass = (type == "ok") ? "banner ok" : (type == "danger" ? "banner danger" : "banner");
    String banner;
    banner.reserve(400);
    banner += "<div class='" + klass + "'>";
    banner += "<div style='font-weight:700;margin-bottom:4px'>" + htmlEscape(title) + "</div>";
    if (details.length() > 0) {
        banner += "<div style='color:var(--muted)'>" + details + "</div>";
    }
    banner += "</div>";

    apServer.send(200, "text/html", buildConfigPageHtml(banner));
}
} // namespace

void startAccessPointWebServer() {
    // Define custom IP configuration for AP
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        Serial.println("Failed to configure AP IP");
        return;
    }

    // Start the access point
    WiFi.softAP("ap_WindClock", "windClock");

    Serial.println("Access Point started: ap_WindClock");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    // Routes
    apServer.on("/", HTTP_GET, []() {
        apServer.send(200, "text/html", buildConfigPageHtml(""));
    });

    // Legacy endpoint (older links)
    apServer.on("/submit", HTTP_GET, []() {
        apServer.sendHeader("Location", "/", true);
        apServer.send(302, "text/plain", "");
    });

    apServer.on("/save", HTTP_POST, []() {
        const String ssid = apServer.arg("ssid");
        const String password = apServer.arg("password");

        const String url = apServer.arg("url");
        const String station = apServer.arg("station");
        const String knopenMinStr = apServer.arg("knopen_min");

        const String mqttHost = apServer.arg("mqtt_host");
        const String mqttPortStr = apServer.arg("mqtt_port");
        const String mqttUser = apServer.arg("mqtt_user");
        const String mqttPassword = apServer.arg("mqtt_password");
        const String mqttBase = apServer.arg("mqtt_base");

        const bool ledsOn = apServer.hasArg("leds_on");
        const String brightnessStr = apServer.arg("brightness");

        // Persist WiFi
        if (ssid.length() > 0) {
            strncpy(settings.WifiSSID, ssid.c_str(), sizeof(settings.WifiSSID));
            settings.WifiSSID[sizeof(settings.WifiSSID) - 1] = '\0';
        }
        if (password.length() > 0) {
            strncpy(settings.WifiPWD, password.c_str(), sizeof(settings.WifiPWD));
            settings.WifiPWD[sizeof(settings.WifiPWD) - 1] = '\0';
        }

        // Persist wind settings
        if (url.length() > 0) {
            String finalUrl = url;
            if (station.length() > 0) {
                finalUrl = replaceOrAppendStationParam(finalUrl, station);
            }
            finalUrl.toCharArray(settings.url, sizeof(settings.url));
        } else if (station.length() > 0) {
            // If only station was set, try to update existing URL
            String finalUrl = replaceOrAppendStationParam(String(settings.url), station);
            finalUrl.toCharArray(settings.url, sizeof(settings.url));
        }

        int knopenMin = knopenMinStr.toInt();
        if (knopenMin < 0) knopenMin = 0;
        if (knopenMin > 60) knopenMin = 60;
        if (knopenMinStr.length() > 0) {
            settings.KnopenMin = knopenMin;
        }

        // Persist MQTT settings
        if (mqttHost.length() > 0) {
            strncpy(settings.MqttHost, mqttHost.c_str(), sizeof(settings.MqttHost));
            settings.MqttHost[sizeof(settings.MqttHost) - 1] = '\0';
        } else {
            settings.MqttHost[0] = '\0';
        }

        int mqttPort = mqttPortStr.toInt();
        if (mqttPort <= 0 || mqttPort > 65535) mqttPort = 1883;
        settings.MqttPort = mqttPort;

        strncpy(settings.MqttUser, mqttUser.c_str(), sizeof(settings.MqttUser));
        settings.MqttUser[sizeof(settings.MqttUser) - 1] = '\0';

        if (mqttPassword.length() > 0) {
            strncpy(settings.MqttPassword, mqttPassword.c_str(), sizeof(settings.MqttPassword));
            settings.MqttPassword[sizeof(settings.MqttPassword) - 1] = '\0';
        }

        if (mqttBase.length() > 0) {
            strncpy(settings.MqttBaseTopic, mqttBase.c_str(), sizeof(settings.MqttBaseTopic));
            settings.MqttBaseTopic[sizeof(settings.MqttBaseTopic) - 1] = '\0';
        } else if (strlen(settings.MqttBaseTopic) == 0) {
            strncpy(settings.MqttBaseTopic, "windclock", sizeof(settings.MqttBaseTopic));
            settings.MqttBaseTopic[sizeof(settings.MqttBaseTopic) - 1] = '\0';
        }

        // Persist LED settings
        settings.LedsOn = ledsOn;
        int brightness = brightnessStr.toInt();
        if (brightness < 0) brightness = 0;
        if (brightness > 255) brightness = 255;
        if (brightnessStr.length() > 0) {
            settings.Brightness = brightness;
        }

        saveSettingsToNVS();

        // If SSID is set, attempt WiFi connection.
        if (ssid.length() == 0) {
            sendMessageBanner("ok", "Settings saved", "WiFi SSID unchanged. You can now close this page.");
            return;
        }

        Serial.println("Received configuration via AP page.");
        Serial.print("SSID: ");
        Serial.println(ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(settings.WifiSSID, settings.WifiPWD);

        int maxRetries = 20;
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
            delay(500);
            Serial.print(".");
            retries++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to new WiFi:");
            Serial.println(WiFi.localIP());
            saveSettingsToNVS();

            String details;
            details.reserve(160);
            details += "Connected. IP: <small class='code'>";
            details += WiFi.localIP().toString();
            details += "</small>. Home Assistant discovery will work if MQTT is configured.";

            sendMessageBanner("ok", "Connected successfully", details);
        } else {
            Serial.println("\nFailed to connect to provided WiFi.");
            sendMessageBanner("danger", "Saved, but WiFi connection failed", "Double-check SSID/password and try again.");
        }
    });

    apServer.onNotFound([]() {
        apServer.sendHeader("Location", "/", true);
        apServer.send(302, "text/plain", "");
    });

    apServer.begin();
    Serial.println("Access Point WebServer started on port 80");
}

void handleAccessPointClient() {
    apServer.handleClient();
}
