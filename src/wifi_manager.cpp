#include "wifi_manager.h"
#include "ble_manager.h"

static unsigned long lastWiFiAttempt = 0;
static String lastSSID = "";
static String lastPassword = "";

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

bool connectWiFi(String ssid, String password)
{
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("    ╔═══════════════════════════════════════════════════╗");
    Serial.println("    ║                  WiFi: Connecting...              ║");
    Serial.println("    ╚═══════════════════════════════════════════════════╝");

    // Wait for connection (max 15 seconds)
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    {
        delay(100);
        yield();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.println("    ╔═══════════════════════════════════════════════════╗");
        Serial.println("    ║           ✓ WiFi Connected Successfully ✓         ║");
        Serial.println("    ╚═══════════════════════════════════════════════════╝");
        Serial.printf("    ◽ IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.println();
        return true;
    }
    else
    {
        Serial.println("WiFi: Connection failed!");
        return false;
    }
}

bool connectWiFiNonBlocking(String ssid, String password)
{
    // If this is a new connection attempt (different SSID/password)
    if (lastSSID != ssid || lastPassword != password)
    {
        lastSSID = ssid;
        lastPassword = password;
        lastWiFiAttempt = millis();
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.println("    ║                  WiFi: Connecting...              ║");
        return false; // Still connecting
    }

    // If already connected, return true
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }

    // If still within timeout window (10 seconds for non-blocking), keep trying
    if (millis() - lastWiFiAttempt < 10000)
    {
        return false; // Still attempting
    }

    // Timeout reached, return false
    Serial.println("WiFi: Non-blocking connection timeout!");
    return false;
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

unsigned long getLastWiFiAttempt()
{
    return lastWiFiAttempt;
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

