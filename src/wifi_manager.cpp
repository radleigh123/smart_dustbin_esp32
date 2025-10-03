#include "wifi_manager.h"
#include <WiFi.h>

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("WiFi: Initialized");
}

bool connectWiFi(String ssid, String password)
{
    Serial.println("WiFi: Connecting...");
    Serial.print("WiFi: SSID: ");
    Serial.println(ssid);

    WiFi.begin(ssid.c_str(), password.c_str());

    // Wait for connection (max 15 seconds)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi: Connected successfully!");
        Serial.print("WiFi: IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("WiFi: Connection failed!");
        return false;
    }
}

bool isWifiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void checkWiFiConnection()
{
    if (!isWifiConnected())
    {
        Serial.println("WiFi: Connection lost");
    }
}

String getWiFiStatus()
{
    switch (WiFi.status())
    {
    case WL_CONNECTED:
        return "Connected - IP: " + WiFi.localIP().toString();
    case WL_NO_SHIELD:
        return "No WiFi shield";
    case WL_IDLE_STATUS:
        return "Idle";
    case WL_NO_SSID_AVAIL:
        return "SSID not available";
    case WL_SCAN_COMPLETED:
        return "Scan completed";
    case WL_CONNECT_FAILED:
        return "Connection failed";
    case WL_CONNECTION_LOST:
        return "Connection lost";
    case WL_DISCONNECTED:
        return "Disconnected";
    default:
        return "Unknown";
    }
}
