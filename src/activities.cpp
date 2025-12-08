#include "activities.h"

const char *ntpServer = "asia.pool.ntp.org";
const long gmtOffset_sec = 0; // Philippines
const int daylightOffset_sec = 0;

void initTime()
{
    setenv("TZ", "PST-8", 1);
    tzset();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }

    // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

String getLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return "";
    }

    // Format into String
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}