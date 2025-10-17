#include "wifi_manager.h"
#include <WiFi.h>
#include <Preferences.h>
#include "ble_manager.h"

Preferences preferences;

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("WiFi: Initialized");
}

String *getPreferences()
{
    static String prefs[4];
    preferences.begin("wifi", true);
    prefs[0] = preferences.getString("ssid", "");
    prefs[1] = preferences.getString("password", "");
    prefs[2] = preferences.getString("userUID", "");
    prefs[3] = preferences.getString("binID", "");
    preferences.end();

    Serial.printf("WiFi: Preferences retrieved\n");
    Serial.printf("WiFi: SSID: %s\n", prefs[0].c_str());
    Serial.printf("WiFi: User UID: %s\n", prefs[2].c_str());
    Serial.printf("WiFi: Bin ID: %s\n", prefs[3].c_str());

    return prefs;
}

void setPreferences(String ssid, String password, String userUID, String binID)
{
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("userUID", userUID);
    preferences.putString("binID", binID);
    preferences.end();

    Serial.printf("WiFi: Preferences set (readOnly=%s)\n", "false");
}

void clearPreferences()
{
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();

    Serial.println("WiFi: Preferences cleared");
}

bool connectWiFi(String ssid, String password)
{
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("WiFi: Connecting...");

    // Wait for connection (max 15 seconds)
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    {
        delay(100);
        Serial.print(".");
        yield();
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi: Connected successfully!");
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
