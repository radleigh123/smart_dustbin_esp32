#include "ble_manager.h"

// BLE Objects
NimBLEServer *pServer = nullptr;
NimBLEService *pService = nullptr;
NimBLECharacteristic *pSSIDChar = nullptr;
NimBLECharacteristic *pPasswordChar = nullptr;
NimBLECharacteristic *pStatusChar = nullptr;

// Credential storage
String receivedSSID = "";
String receivedPassword = "";
bool credentialsReceived = false;
bool deviceConnected = false;

// Server callbacks
class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer)
    {
        deviceConnected = true;
        Serial.println("\n[BLE] Client connected!");
    }

    void onDisconnect(NimBLEServer *pServer)
    {
        deviceConnected = false;
        Serial.println("\n[BLE] Client disconnected!");
        // Restart advertising
        NimBLEDevice::startAdvertising();
        Serial.println("[BLE] Advertising restarted");
    }
};

// SSID Characteristic Callbacks
class SSIDCharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        Serial.println("\n========================================");
        Serial.println("[BLE] SSID Characteristic Write Event!");
        Serial.print("[BLE] Raw length: ");
        Serial.println(value.length());
        Serial.print("[BLE] Raw value (hex): ");
        for (size_t i = 0; i < value.length(); i++)
        {
            Serial.printf("%02X ", (unsigned char)value[i]);
        }
        Serial.println();

        if (value.length() > 0)
        {
            receivedSSID = String(value.c_str());
            Serial.print("[BLE] SSID received: '");
            Serial.print(receivedSSID);
            Serial.println("'");
            Serial.println("========================================\n");

            // Update status
            pStatusChar->setValue("SSID received: " + receivedSSID);
            pStatusChar->notify();
        }
        else
        {
            Serial.println("[BLE] WARNING: Empty SSID received!");
            Serial.println("========================================\n");
        }
    }

    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        Serial.println("[BLE] SSID Read request");
    }
};

// Password Characteristic Callbacks
class PasswordCharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        Serial.println("\n========================================");
        Serial.println("[BLE] Password Characteristic Write Event!");
        Serial.print("[BLE] Raw length: ");
        Serial.println(value.length());

        if (value.length() > 0)
        {
            receivedPassword = String(value.c_str());
            Serial.print("[BLE] Password received: ");
            // Print masked password
            for (size_t i = 0; i < receivedPassword.length(); i++)
            {
                Serial.print("*");
            }
            Serial.println();
            Serial.println("========================================\n");

            // Mark credentials as complete if both are received
            if (receivedSSID.length() > 0)
            {
                credentialsReceived = true;
                Serial.println("\n*** Both credentials received! ***");
                pStatusChar->setValue("Credentials complete!");
                pStatusChar->notify();
            }
            else
            {
                pStatusChar->setValue("Password received, need SSID");
                pStatusChar->notify();
            }
        }
        else
        {
            Serial.println("[BLE] WARNING: Empty password received!");
            Serial.println("========================================\n");
        }
    }

    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        Serial.println("[BLE] Password Read request");
    }
};

void initBLE()
{
    Serial.println("\n[BLE] Initializing BLE...");

    // Initialize NimBLE
    NimBLEDevice::init("SmartDustbin_ESP32");

    // Set power level for better range
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create BLE Service
    pService = pServer->createService(SERVICE_UUID);

    // Create SSID Characteristic (Write only)
    pSSIDChar = pService->createCharacteristic(
        SSID_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pSSIDChar->setCallbacks(new SSIDCharacteristicCallbacks());

    // Create Password Characteristic (Write only)
    pPasswordChar = pService->createCharacteristic(
        PASS_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pPasswordChar->setCallbacks(new PasswordCharacteristicCallbacks());

    // Create Status Characteristic (Read + Notify)
    pStatusChar = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pStatusChar->setValue("Waiting for credentials...");

    // Start the service
    pService->start();

    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setMinInterval(0x20); // 40ms
    pAdvertising->setMaxInterval(0x40); // 100ms

    NimBLEDevice::startAdvertising();

    Serial.println("[BLE] BLE Initialization Complete!");
    Serial.println("[BLE] Device Name: SmartDustbin_ESP32");
    Serial.println("[BLE] Service UUID: " SERVICE_UUID);
    Serial.println("[BLE] SSID Char UUID: " SSID_CHAR_UUID);
    Serial.println("[BLE] Pass Char UUID: " PASS_CHAR_UUID);
    Serial.println("[BLE] Status Char UUID: " STATUS_CHAR_UUID);
    Serial.println("[BLE] Advertising started...\n");
}

bool hasCredentials()
{
    bool hasAll = credentialsReceived &&
                  receivedSSID.length() > 0 &&
                  receivedPassword.length() > 0;

    if (hasAll)
    {
        Serial.println("[BLE] Credentials check: READY");
        Serial.print("[BLE] SSID: '");
        Serial.print(receivedSSID);
        Serial.println("'");
        Serial.print("[BLE] Password length: ");
        Serial.println(receivedPassword.length());
    }

    return hasAll;
}

String getSSID()
{
    return receivedSSID;
}

String getPassword()
{
    return receivedPassword;
}

void clearCredentials()
{
    Serial.println("\n[BLE] Clearing credentials...");
    receivedSSID = "";
    receivedPassword = "";
    credentialsReceived = false;
    updateBLEStatus("Credentials cleared - ready for new credentials");
}

void updateBLEStatus(const char *status)
{
    if (pStatusChar != nullptr)
    {
        Serial.print("[BLE] Status update: ");
        Serial.println(status);
        pStatusChar->setValue(status);
        if (deviceConnected)
        {
            pStatusChar->notify();
        }
    }
}
