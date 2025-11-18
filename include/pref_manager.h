#ifndef PREF_MANAGER_H
#define PREF_MANAGER_H

#include "Arduino.h"
#include "Preferences.h"

void setWifiPref(String ssid, String password, String userUID, String binID);
String *getWifiPref();
void clearWifiPref();

void setBoot();
int getBoot();
void clearBootPref();

#endif