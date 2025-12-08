#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <iostream>

void initWiFi();
bool connectWiFi(String ssid, String password);
bool connectWiFiNonBlocking(String ssid, String password);
bool isWifiConnected();
void checkWiFiConnection();
String getWiFiStatus();
unsigned long getLastWiFiAttempt();

#endif