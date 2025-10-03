#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

void initWiFi();
bool connectWiFi(String ssid, String password);
bool isWifiConnected();
void checkWiFiConnection();
String getWiFiStatus();

#endif