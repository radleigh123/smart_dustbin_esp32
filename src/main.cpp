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
    Serial.println("Step 0: Initializing Servo, IR Sensor, and Ultrasonic Sensor...");

    // Init WiFi
    Serial.println("Step 1: Initializing WiFi and Firebase...");
    initWiFi();
    initFirebase();

    Serial.println("Step 2: Initializing BLE provisioning...");
    initBLE();

    Serial.println("\n========================================");
    Serial.println("System ready!");
    Serial.println("========================================");
    Serial.println("Use NRF Connect app to provision WiFi");
    Serial.println("Device name: SmartDustbin_ESP32");
    Serial.println("========================================\n");
}

void loop()
{
    unsigned long currentMillis = millis();

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

                // Optional: Clear credentials from memory for security
                // clearCredentials();
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
            else
            {
                // Optional: Print status periodically
                Serial.print("WiFi Status: ");
                Serial.println(getWiFiStatus());
            }
        }

        if (wifiConnected)
        {
            firebaseLoop();

            if (firebaseReady())
            {
                static unsigned long lastSendMillis = 0;
                unsigned long sendInterval = 10000; // Send data every 10 seconds
                if (currentMillis - lastSendMillis >= sendInterval)
                {
                    lastSendMillis = currentMillis;
                    float distance = getDistance(currentMillis);
                    firebaseUpdateUltrasonicData(distance);
                    Serial.printf("Distance: %.2f cm\n", distance);
                }
            }
        }
    }

    controlLidAuto(currentMillis);
}
