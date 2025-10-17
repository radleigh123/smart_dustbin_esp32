#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <iostream>

void initWiFi();
String *getPreferences();
void setPreferences(String ssid, String password, String userUID, String binID);
void clearPreferences();
bool connectWiFi(String ssid, String password);
bool isWifiConnected();
void checkWiFiConnection();
String getWiFiStatus();

#endif