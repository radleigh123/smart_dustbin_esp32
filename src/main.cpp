#include <Arduino.h>
#include <WiFi.h>
#include "ble_manager.h"
#include "wifi_manager.h"
#include "servo_controller.h"
#include "ir_sensor.h"
#include "ultrasonic_sensor.h"
#include "firebase_app.h"

// State variables
static unsigned long lastMainLoopMillis = 0;
const unsigned long MAIN_LOOP_INTERVAL = 100; // 100ms interval

static bool wifiConnected = false;
static bool provisioningComplete = false;
static unsigned long lastWiFiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10000;

static String ssid = "";
static String password = "";
static String userUID = "";
static String binID = "";

void setup(void)
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n========================================");
    Serial.println("   Smart Dustbin ESP32 - Starting");
    Serial.println("========================================\n");

    // Init Servo, IR Sensor, Ultrasonic Sensor
    initServo();
    initIRSensor();
    initUltrasonicSensor();
    Serial.println("\n========================================");
    Serial.println("   Initializing Servo, IR Sensor, and Ultrasonic Sensor...");
    Serial.println("========================================");

    // Init WiFi
    Serial.println("\n========================================");
    Serial.println("   Initializing WiFi");
    Serial.println("========================================");
    initWiFi();

    static String *savedPrefs = getPreferences();
    ssid = savedPrefs[0];
    password = savedPrefs[1];
    userUID = savedPrefs[2];
    binID = savedPrefs[3];

    if (ssid.length() > 0 && password.length() > 0 && userUID.length() > 0 && binID.length() > 0)
    {
        wifiConnected = connectWiFi(ssid, password);

        if (wifiConnected)
        {
            Serial.println("Saved WiFi Connected Successfully!");
            updateBLEStatus("WiFi Connected!");

            // Init Firebase
            Serial.println("\n========================================");
            Serial.println("   Initializing Firebase");
            Serial.println("========================================");
            while (WiFi.status() != WL_CONNECTED)
                delay(500);
            delay(2000);
            initFirebase();

            // This ensures subscription runs only after Firebase is ready
            while (!firebaseReady())
            {
                firebaseLoop();
                delay(10);
            }

            provisioningComplete = true;

            // SUBSCRIBE
            firebaseSetPath(userUID + "/" + binID);
            delay(1200);
        }
        else
        {
            Serial.println("Saved WiFi Connection Failed!");
            updateBLEStatus("Saved WiFi re-provisioning needed");
        }
    }

    if (!(wifiConnected && provisioningComplete))
    {
        Serial.println("Initialize BLE for re-provisioning...");
        initBLE();
        Serial.println("\n========================================");
        Serial.println("System ready!");
        Serial.println("========================================");
        Serial.println("Use NRF Connect app to provision WiFi");
        Serial.println("Device name: SmartDustbin_ESP32");
        Serial.println("========================================\n");
    }
}

void loop()
{
    unsigned long currentMillis = millis();
    firebaseLoop();

    if (currentMillis - lastMainLoopMillis >= MAIN_LOOP_INTERVAL)
    {
        if (hasCredentials() && !provisioningComplete)
        {
            Serial.println("\n----------------------------------------");
            Serial.println("WiFi credentials received!");
            Serial.println("Attempting to connect to WiFi...");
            Serial.println("----------------------------------------");

            updateBLEStatus("Connecting to WiFi...");

            wifiConnected = connectWiFi(getSSID(), getPassword());

            if (wifiConnected)
            {
                Serial.println("\n✓ WiFi Connected Successfully!");
                Serial.print("✓ IP Address: ");
                Serial.println(WiFi.localIP());
                updateBLEStatus("WiFi Connected!");
                provisioningComplete = true;

                Serial.println("\n========================================");
                Serial.println("   Initializing Firebase");
                Serial.println("========================================");
                while (WiFi.status() != WL_CONNECTED)
                    delay(500);
                delay(2000);
                initFirebase();

                // Set Firebase path
                firebaseSetPath(getUserUID() + "/" + getBinID());
                firebaseSetData("", "");

                // Saving to NVS
                setPreferences(getSSID(), getPassword(), getUserUID(), getBinID());
            }
            else
            {
                Serial.println("\n✗ WiFi Connection Failed!");
                Serial.println("Please check your credentials and try again.");
                updateBLEStatus("WiFi connection failed");

                // Clear credentials so user can try again
                clearCredentials();
                provisioningComplete = false;
            }
        }

        if (wifiConnected && (millis() - lastWiFiCheck > WIFI_CHECK_INTERVAL))
        {
            lastWiFiCheck = millis();
            checkWiFiConnection();

            if (!isWifiConnected())
            {
                Serial.println("WiFi connection lost! Reconnecting...");
                wifiConnected = false;
                provisioningComplete = false;
                updateBLEStatus("WiFi disconnected");
            }
        }

        if (wifiConnected)
        {
            if (firebaseReady())
            {
                static unsigned long lastSendMillis = 0;
                unsigned long sendInterval = 10000; // Send data every 10 seconds
                if (currentMillis - lastSendMillis >= sendInterval)
                {
                    lastSendMillis = currentMillis;
                    float distance = getDistance(currentMillis);
                    firebaseUpdateUltrasonicData(distance);
                    firebaseUpdateFillLevel(distance);
                }
            }
        }
    }
    controlLidAuto(currentMillis);
}
