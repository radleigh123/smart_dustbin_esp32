#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

// BLE Service UUIDs (Nordic UART Service compatible)
#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define SSID_CHAR_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define PASS_CHAR_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define STATUS_CHAR_UUID "6e400004-b5a3-f393-e0a9-e50e24dcca9e"
#define WIFI_SCAN_CHAR_UUID "6e400005-b5a3-f393-e0a9-e50e24dcca9e"

// BLE Manager Functions
void initBLE();
void updateBLEStatus(const char *status);
bool isDeviceConnected();
String getSSID();
String getPassword();
bool hasCredentials();
void clearCredentials();
String scanWifiNetworks();

#endif