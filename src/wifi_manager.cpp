#include <WiFi.h>

const char *ssid = "your_SSID";
const char *password = "your_PASSWORD";

void initWifiProvisioning()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

bool isWifiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}
