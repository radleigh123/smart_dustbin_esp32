#include "ble_manager.h"
#include "wifi_manager.h"

// BLE Server and Characteristics
static NimBLEServer *pServer = nullptr;
static NimBLECharacteristic *pSSIDCharacteristic = nullptr;
static NimBLECharacteristic *pPasswordCharacteristic = nullptr;
static NimBLECharacteristic *pStatusCharacteristic = nullptr;

// WiFi Credentials Storage
static String wifiSSID = "";
static String wifiPassword = "";
static bool credentialsReceived = false;
static bool deviceConnected = false;

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
 * Characteristic Callbacks - Handle read/write operations
 */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
    {
        std::string uuid = pCharacteristic->getUUID().toString();
        std::string value = pCharacteristic->getValue();

        Serial.printf("BLE: Write to %s\n", uuid.c_str());

        // SSID write
        if (uuid == SSID_CHAR_UUID)
        {
            wifiSSID = String(value.c_str());
            Serial.print("BLE: SSID received: ");
            Serial.println(wifiSSID);
            updateBLEStatus("SSID received");
        }
        // Password write
        else if (uuid == PASS_CHAR_UUID)
        {
            wifiPassword = String(value.c_str());
            Serial.println("BLE: Password received");
            updateBLEStatus("Credentials received");

            if (wifiSSID.length() > 0 && wifiPassword.length() > 0)
            {
                credentialsReceived = true;
                Serial.println("BLE: Complete credentials received - ready to connect");
            }
        }
    }

    void onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override
    {
        Serial.printf("BLE: Read from %s\n", pCharacteristic->getUUID().toString().c_str());
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

    // Set security (optional - uncomment if you want pairing)
    // NimBLEDevice::setSecurityAuth(false, false, true);

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
