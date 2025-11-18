#include "ble_manager.h"
#include "wifi_manager.h"
#include "servo_controller.h"
#include "ir_sensor.h"
#include "ultrasonic_sensor.h"
#include "firebase_app.h"
#include "pref_manager.h"

// State variables
static unsigned long lastMainLoopMillis = 0;
const unsigned long MAIN_LOOP_INTERVAL = 100; // 100ms interval

static bool wifiConnected = false;
// Prevents loop on initializing firebase
static bool provisioningComplete = false;
static unsigned long lastWiFiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10000;

// Prevents loop on initializing firebase data
static bool initFirebaseRTDB = false;

static String *savedPrefs;
static String ssid = "";
static String password = "";
static String userUID = "";
static String binID = "";

String currentCmd = "";

void setup(void)
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n");
    Serial.println("    ╔═══════════════════════════════════════════════════╗");
    Serial.println("    ║           Smart Dustbin ESP32 - Starting          ║");
    Serial.println("    ╠═══════════════════════════════════════════════════╣");

    // Init Servo, IR Sensor, Ultrasonic Sensor
    initServo();
    initIRSensor();
    initUltrasonicSensor();
    Serial.println("    ║  Initializing the following:                      ║");
    Serial.println("    ║     + Servo module                                ║");
    Serial.println("    ║     + IR Sensor module                            ║");
    Serial.println("    ║     + Ultrasonic Sensor module                    ║");
    Serial.println("    ╠═══════════════════════════════════════════════════╣");

    // Init WiFi
    Serial.println("    ║                 Initializing WiFi...              ║");
    Serial.println("    ╚═══════════════════════════════════════════════════╝");
    Serial.println("\n");
    savedPrefs = getWifiPref();
    ssid = savedPrefs[0];
    password = savedPrefs[1];
    userUID = savedPrefs[2];
    binID = savedPrefs[3];
    initWiFi();

    if (ssid.length() > 0 && password.length() > 0 && userUID.length() > 0 && binID.length() > 0)
    {
        wifiConnected = connectWiFi(ssid, password);

        if (wifiConnected)
        {
            // Init Firebase
            Serial.println("\n");
            Serial.println("    ╔═══════════════════════════════════════════════════╗");
            Serial.println("    ║               Initializing Firebase...            ║");
            Serial.println("    ╚═══════════════════════════════════════════════════╝");
            Serial.println("\n");
            while (WiFi.status() != WL_CONNECTED)
                delay(500);
            delay(2000);
            initFirebase("trash_bins/" + userUID + "/" + binID + "/");

            // This ensures subscription runs only after Firebase is ready
            while (!firebaseReady())
            {
                firebaseLoop();
                delay(10);
            }

            provisioningComplete = true;

            firebaseSetPath(userUID + "/" + binID);
            // firebaseSetData("", "");
        }
        else
        {
            Serial.println("Saved WiFi re-provisioning needed!");
        }
    }

    if (!(wifiConnected && provisioningComplete))
    {
        clearWifiPref();
        clearBootPref();
        initBLE();
        Serial.println("\n");
        Serial.println("    ╔═══════════════════════════════════════════════════╗");
        Serial.println("    ║                   Initialize BLE...               ║");
        Serial.println("    ╠═══════════════════════════════════════════════════╣");
        Serial.println("    ║        Use NRF Connect app to provision WiFi      ║");
        Serial.println("    ║           Device name: SmartDustbin_ESP32         ║");
        Serial.println("    ╚═══════════════════════════════════════════════════╝");
        Serial.println("\n");
    }
}

void loop()
{
    unsigned long currentMillis = millis();
    firebaseLoop();

    // Servo mode: AUTO (default) | MANUAL
    currentCmd = getCommand();
    if (currentCmd == "auto")
    {
        controlLidAuto(currentMillis);
    }
    else
    {
        if (currentCmd == "open")
            openLid();
        else
            closeLid();
    }

    if (currentMillis - lastMainLoopMillis >= MAIN_LOOP_INTERVAL)
    {
        if (hasCredentials())
        {
            wifiConnected = connectWiFi(getSSID(), getPassword());

            if (wifiConnected)
            {
                Serial.println();
                Serial.println("    ╔═══════════════════════════════════════════════════╗");
                Serial.println("    ║           Initializing Firebase (loop)...         ║");
                Serial.println("    ╚═══════════════════════════════════════════════════╝");
                Serial.println();
                while (WiFi.status() != WL_CONNECTED)
                    delay(500);

                // Saving to NVS
                ssid = getSSID();
                password = getPassword();
                userUID = getUserUID();
                binID = getBinID();
                // Serial.printf("ssid=%s, password=%s, userUid=%s, binId=%s\n", ssid, password, userUID.c_str(), binID.c_str());
                setWifiPref(ssid, password, userUID, binID);
                delay(1200);
                savedPrefs = getWifiPref();
                ssid = savedPrefs[0];
                password = savedPrefs[1];
                userUID = savedPrefs[2];
                binID = savedPrefs[3];

                provisioningComplete = true; // TODO: maybe refactor because it has the same purpose as `initFirebaseRTDB`

                // Set Firebase path
                firebaseSetPath(userUID + "/" + binID);

                initFirebase("trash_bins/" + userUID + "/" + binID + "/");
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

        // Checks to see if firebase has already initialized
        if (wifiConnected && firebaseReady() && !initFirebaseRTDB)
        {
            String *firebaseData = getfirebaseData();

            // This ensures existing dustbin's data will not be overwritten
            if (!(firebaseData[0] == "null" || firebaseData[1] == "null"))
            {
                Serial.println("No need for initial data");
                firebaseSubscribe();
                initFirebaseRTDB = true;
                return;
            }

            firebaseSetData("", "");
            firebaseSubscribe();
            initFirebaseRTDB = true;
        }

        if (wifiConnected && (millis() - lastWiFiCheck > WIFI_CHECK_INTERVAL))
        {
            lastWiFiCheck = millis();
            checkWiFiConnection();

            if (!isWifiConnected())
            {
                Serial.println("WiFi connection lost! Reconnecting...");
                wifiConnected = connectWiFi(ssid, password);
            }
        }

        if (wifiConnected)
        {
            // Stops ultrasonic from updating the distance if lid is open
            if (currentCmd == "open")
                return;

            if (firebaseReady())
            {
                static unsigned long lastSendMillis = 0;
                unsigned long sendInterval = 10000; // Send data every 10 seconds
                if (currentMillis - lastSendMillis >= sendInterval)
                {
                    lastSendMillis = currentMillis;
                    float distance = getDistance(currentMillis);
                    firebaseUpdateUltrasonicData(distance);
                }
            }
        }
    }
}
