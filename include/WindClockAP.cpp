#include <WiFi.h>
#include <WebServer.h>
#include "ClockSettings.h"

WebServer apServer(80);

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

    // Optionally print the assigned IP
    Serial.println("Access Point started: ap_WindClock");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    // Define web server routes
    apServer.on("/", HTTP_GET, []() {
        apServer.send(200, "text/html", R"rawliteral(
            <!DOCTYPE html>
            <html>
            <head><title>Configure WiFi</title></head>
            <body>
                <h1>WindClock WiFi Setup</h1>
                <form action="/submit" method="get">
                    SSID: <input type="text" name="ssid"><br>
                    Password: <input type="password" name="password"><br><br>
                    <input type="submit" value="Save">
                </form>
            </body>
            </html>
        )rawliteral");
    });

    apServer.on("/submit", HTTP_GET, []() {
        String ssid = apServer.arg("ssid");
        String password = apServer.arg("password");

        Serial.println("Received WiFi credentials:");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(password);

        // Attempt to connect to the submitted WiFi credentials
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

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
            strncpy(settings.WifiSSID, ssid.c_str(), sizeof(settings.WifiSSID));
            strncpy(settings.WifiPWD, password.c_str(), sizeof(settings.WifiPWD));
            saveSettingsToNVS();
            apServer.send(200, "text/html", "<h1>Connected successfully! You can now close this page.</h1>");
        } else {
            Serial.println("\nFailed to connect to provided WiFi.");
            apServer.send(200, "text/html", "<h1>Failed to connect to WiFi. Please try again.</h1>");
        }
    });

    apServer.begin();
    Serial.println("Access Point WebServer started on port 80");
}

void handleAccessPointClient() {
    apServer.handleClient();
}
