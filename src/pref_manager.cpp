#include "pref_manager.h"

Preferences pref;
int bootCount = 0;

void setWifiPref(String ssid, String password, String userUID, String binID)
{
    pref.begin("wifi", false);
    pref.putString("ssid", ssid);
    pref.putString("password", password);
    pref.putString("userUID", userUID);
    pref.putString("binID", binID);
    pref.end();

    Serial.println();
    Serial.println("    ╔═══════════════════════════════════════════════════╗");
    Serial.println("    ║          Preferences set (readOnly=FALSE)         ║");
    Serial.println("    ╚═══════════════════════════════════════════════════╝");
    Serial.println();
}

String *getWifiPref()
{
    static String prefs[4];
    pref.begin("wifi", true);
    prefs[0] = pref.getString("ssid", "");
    prefs[1] = pref.getString("password", "");
    prefs[2] = pref.getString("userUID", "");
    prefs[3] = pref.getString("binID", "");
    pref.end();

    Serial.println();
    Serial.println("    ╔═══════════════════════════════════════════════════╗");
    Serial.println("    ║               Preferences retrieved               ║");
    Serial.println("    ╚═══════════════════════════════════════════════════╝");
    Serial.printf("    ◽ PREF: SSID: %s\n", prefs[0].c_str());
    // Serial.printf("    ◽ PREF: PASSWORD: %s\n", prefs[1].c_str()); // Display not included for security purposes
    Serial.printf("    ◽ PREF: User UID: %s\n", prefs[2].c_str());
    Serial.printf("    ◽ PREF: Bin ID: %s\n", prefs[3].c_str());
    Serial.println();

    return prefs;
}

void clearWifiPref()
{
    pref.begin("wifi", false);
    pref.clear();
    pref.end();

    Serial.println();
    Serial.println("    ╔═══════════════════════════════════════════════════╗");
    Serial.println("    ║              Preferences=wifi deleted             ║");
    Serial.println("    ╚═══════════════════════════════════════════════════╝");
    Serial.println();
}

void setBoot()
{
    pref.begin("boot", false);
    bootCount = pref.getInt("bootCount", 0);
    bootCount++;
    pref.putInt("bootCount", bootCount);
}

int getBoot()
{
    pref.begin("boot", false);
    return pref.getInt("bootCount", 0);
}

void clearBootPref()
{
    pref.begin("boot", false);
    pref.clear();
    pref.end();

    Serial.println();
    Serial.println("    ╔═══════════════════════════════════════════════════╗");
    Serial.println("    ║              Preferences=boot deleted             ║");
    Serial.println("    ╚═══════════════════════════════════════════════════╝");
    Serial.println();
}

void setProvisioningAttempts(int attempts)
{
    pref.begin("provisioning", false);
    pref.putInt("attempts", attempts);
    pref.end();
}

int getProvisioningAttempts()
{
    pref.begin("provisioning", true);
    int attempts = pref.getInt("attempts", 0);
    pref.end();
    return attempts;
}

void resetProvisioningAttempts()
{
    pref.begin("provisioning", false);
    pref.putInt("attempts", 0);
    pref.end();
}