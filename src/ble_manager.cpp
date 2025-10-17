#include "ble_manager.h"
#include "wifi_manager.h"
#include "servo_controller.h"
#include <WiFi.h>

// BLE Server and Characteristics
static NimBLEServer *pServer = nullptr;
static NimBLECharacteristic *pSSIDCharacteristic = nullptr;
static NimBLECharacteristic *pPasswordCharacteristic = nullptr;
static NimBLECharacteristic *pStatusCharacteristic = nullptr;
static NimBLECharacteristic *pWifiScanCharacteristic = nullptr;
static NimBLECharacteristic *pServoCharacteristic = nullptr;
static NimBLECharacteristic *pUserUIDCharacteristic = nullptr;
static NimBLECharacteristic *pBinIDCharacteristic = nullptr;

// WiFi Credentials Storage
static String wifiSSID = "";
static String wifiPassword = "";
static bool credentialsReceived = false;
static bool deviceConnected = false;

static String userUID = "";
static String binID = "";

/**
 * Server Callbacks - Handle BLE connection events
 */
class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
    {
        deviceConnected = true;
        Serial.println("BLE: Client connected");
        Serial.printf("BLE: Client address: %s\n", connInfo.getAddress().toString().c_str());

        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);

        updateBLEStatus("Connected");
    }

    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override
    {
        deviceConnected = false;
        Serial.println("BLE: Client disconnected");
        Serial.println("BLE: Restarting advertising...");
        NimBLEDevice::startAdvertising();
    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo &connInfo) override
    {
        Serial.printf("BLE: MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }
};

/**
 * Scan for WiFi networks and return comma-separated list
 */
String scanWifiNetworks()
{
    Serial.println("BLE: Starting WiFi scan...");

    int n = WiFi.scanNetworks();
    String networks = "";

    if (n == 0)
    {
        Serial.println("BLE: No networks found");
        return "No networks found";
    }

    Serial.printf("BLE: Found %d networks\n", n);

    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            networks += ",";
        networks += WiFi.SSID(i);
        Serial.printf("  %d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }

    return networks;
}

/**
 * Characteristic Callbacks - Handle read/write operations
 */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
    {
        std::string value = pCharacteristic->getValue();

        if (pCharacteristic->getUUID().equals(NimBLEUUID(SSID_CHAR_UUID)))
        {
            wifiSSID = String(value.c_str());
            Serial.printf("BLE: SSID received: %s\n", wifiSSID.c_str());
            updateBLEStatus("SSID received");
        }
        else if (pCharacteristic->getUUID().equals(NimBLEUUID(PASS_CHAR_UUID)))
        {
            wifiPassword = String(value.c_str());
            Serial.println("BLE: Password received (hidden for security)");
            updateBLEStatus("Credentials received");

            credentialsReceived = true;

            if (hasCredentials())
            {
                Serial.println("BLE: Complete credentials received - ready to connect");
            }
        }
        else if (pCharacteristic->getUUID().equals(NimBLEUUID(SERVO_CHAR_UUID)))
        {
            int angle = 0;
            if (value.length() > 0)
            {
                angle = atoi(value.c_str());
                if (angle < 0)
                    angle = 0;
                if (angle > 180)
                    angle = 180;
                controlServo(angle);
                Serial.printf("BLE: Servo angle set to %d degrees\n", angle);
                updateBLEStatus("Servo angle set");
            }
        }
        else if (pCharacteristic->getUUID().equals(NimBLEUUID(USERUID_CHAR_UUID)))
        {
            userUID = String(value.c_str());
            Serial.printf("BLE: User UID received: %s\n", userUID.c_str());
            updateBLEStatus("User UID received");
        }
        else if (pCharacteristic->getUUID().equals(NimBLEUUID(BINID_CHAR_UUID)))
        {
            binID = String(value.c_str());
            Serial.printf("BLE: Bin ID received: %s\n", binID.c_str());
            updateBLEStatus("Bin ID received");
        }
    }

    void onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
    {
        Serial.printf("BLE: Read from %s\n", pCharacteristic->getUUID().toString().c_str());

        // If WiFi scan characteristic is read, perform scan
        if (pCharacteristic->getUUID().equals(NimBLEUUID(WIFI_SCAN_CHAR_UUID)))
        {
            String networks = scanWifiNetworks();
            pCharacteristic->setValue(networks.c_str());
            Serial.printf("BLE: Sent WiFi networks: %s\n", networks.c_str());
        }
    }

    void onStatus(NimBLECharacteristic *pCharacteristic, int code) override
    {
        Serial.printf("BLE: Notification/Indication status: %d, %s\n",
                      code, NimBLEUtils::returnCodeToString(code));
    }

    void onSubscribe(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo, uint16_t subValue) override
    {
        std::string str = "BLE: Client ID: ";
        str += std::to_string(connInfo.getConnHandle());
        str += " Address: ";
        str += connInfo.getAddress().toString();

        if (subValue == 0)
        {
            str += " unsubscribed from ";
        }
        else if (subValue == 1)
        {
            str += " subscribed to notifications for ";
        }
        else if (subValue == 2)
        {
            str += " subscribed to indications for ";
        }

        str += std::string(pCharacteristic->getUUID());
        Serial.println(str.c_str());
    }
};

static ServerCallbacks serverCallbacks;
static CharacteristicCallbacks chrCallbacks;

/**
 * Initialize BLE Server with WiFi provisioning service
 */
void initBLE()
{
    Serial.println("BLE: Initializing...");

    NimBLEDevice::init("SmartDustbin_ESP32");

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks);

    // Create WiFi Provisioning Service (Nordic UART Service UUID)
    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    // Create SSID Characteristic (Write only)
    pSSIDCharacteristic = pService->createCharacteristic(
        SSID_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pSSIDCharacteristic->setCallbacks(&chrCallbacks);

    // Create Password Characteristic (Write only)
    pPasswordCharacteristic = pService->createCharacteristic(
        PASS_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pPasswordCharacteristic->setCallbacks(&chrCallbacks);

    // Create Status Characteristic (Read + Notify)
    pStatusCharacteristic = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pStatusCharacteristic->setValue("Ready for provisioning");
    pStatusCharacteristic->setCallbacks(&chrCallbacks);

    // ADD: Create WiFi Scan Characteristic (Read only)
    pWifiScanCharacteristic = pService->createCharacteristic(
        WIFI_SCAN_CHAR_UUID,
        NIMBLE_PROPERTY::READ);
    pWifiScanCharacteristic->setValue("Scan not started");
    pWifiScanCharacteristic->setCallbacks(&chrCallbacks);

    // ADD: Create Servo Control Characteristic (Write only)
    pServoCharacteristic = pService->createCharacteristic(
        SERVO_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pServoCharacteristic->setCallbacks(&chrCallbacks);

    // ADD: Create User UID Characteristic (Write only)
    pUserUIDCharacteristic = pService->createCharacteristic(
        USERUID_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pUserUIDCharacteristic->setCallbacks(&chrCallbacks);

    // ADD: Create Bin ID Characteristic (Write only)
    pBinIDCharacteristic = pService->createCharacteristic(
        BINID_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pBinIDCharacteristic->setCallbacks(&chrCallbacks);

    // Start the service
    pService->start();

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("SmartDustbin_ESP32");
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.println("BLE: Initialized successfully");
    Serial.println("BLE: Advertising started - Device name: SmartDustbin_ESP32");
    Serial.println("BLE: Waiting for WiFi credentials...");
}

/**
 * Update BLE status characteristic and send notification
 */
void updateBLEStatus(const char *status)
{
    if (pStatusCharacteristic)
    {
        pStatusCharacteristic->setValue(status);

        // Send notification if device is connected
        if (deviceConnected)
        {
            pStatusCharacteristic->notify();
        }

        Serial.printf("BLE: Status updated: %s\n", status);
    }
}

/**
 * Check if a BLE device is connected
 */
bool isDeviceConnected()
{
    return deviceConnected;
}

/**
 * Get received SSID
 */
String getSSID()
{
    return wifiSSID;
}

/**
 * Get received password
 */
String getPassword()
{
    return wifiPassword;
}

/**
 * Get received User UID
 */
String getUserUID()
{
    return userUID;
}

/**
 * Get received Bin ID
 */
String getBinID()
{
    return binID;
}

/**
 * Check if complete credentials have been received
 */
bool hasCredentials()
{
    return credentialsReceived && wifiSSID.length() > 0 && wifiPassword.length() > 0;
}

/**
 * Clear stored credentials
 */
void clearCredentials()
{
    wifiSSID = "";
    wifiPassword = "";
    credentialsReceived = false;
    Serial.println("BLE: Credentials cleared");
}
