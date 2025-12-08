#include "ble_manager.h"
#include "wifi_manager.h"
#include "servo_controller.h"
#include "ir_sensor.h"
#include "ultrasonic_sensor.h"
#include "firebase_app.h"
#include "pref_manager.h"
#include "activities.h"

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
static bool isDestroyed = false;

// Provisioning retry management
static unsigned long lastProvisioningAttempt = 0;
const unsigned long PROVISIONING_RETRY_INTERVAL = 30000; // Retry every 30 seconds
const int MAX_PROVISIONING_ATTEMPTS = 5;                 // Clear credentials after 5 failed attempts

static String *savedPrefs;
static String ssid = "";
static String password = "";
static String userUID = "";
static String binID = "";

String currentCmd = "";
String currentTask = "";
int currentAngle = -1;

unsigned long closeTimestamp = 0;
const unsigned long AUTO_TIMEOUT = 30000;

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
            // Reset provisioning attempt counter on successful connection
            resetProvisioningAttempts();

            // Init Firebase
            Serial.println("\n");
            Serial.println("    ╔═══════════════════════════════════════════════════╗");
            Serial.println("    ║               Initializing Firebase...            ║");
            Serial.println("    ╚═══════════════════════════════════════════════════╝");
            Serial.println("\n");
            while (WiFi.status() != WL_CONNECTED)
                delay(500);
            delay(2000);

            firebaseSetPath(userUID, binID);
            initFirebase();

            // Init Time
            initTime();

            // This ensures subscription runs only after Firebase is ready
            unsigned long firebaseWaitStart = millis();
            while (!firebaseReady())
            {
                firebaseLoop();
                delay(10);
                // Timeout: 30 seconds for Firebase initialization
                if (millis() - firebaseWaitStart > 30000)
                {
                    Serial.println("Firebase initialization timeout!");
                    break;
                }
            }

            provisioningComplete = true;
        }
        else
        {
            Serial.println("Saved WiFi credentials exist but connection failed.");
            Serial.println("Device will retry connection in main loop.");
        }
    }

    if (!(wifiConnected && provisioningComplete))
    {
        // Only clear credentials if they've failed too many times
        if (hasCredentials() && getProvisioningAttempts() >= MAX_PROVISIONING_ATTEMPTS)
        {
            Serial.println("\n");
            Serial.println("    ╔═══════════════════════════════════════════════════╗");
            Serial.println("    ║   Max provisioning attempts exceeded. Clearing    ║");
            Serial.println("    ║          credentials for re-provisioning          ║");
            Serial.println("    ╚═══════════════════════════════════════════════════╝");
            Serial.println("\n");
            clearWifiPref();
            clearBootPref();
            resetProvisioningAttempts();
        }
        else if (hasCredentials())
        {
            // Credentials exist but connection failed - increment attempt counter
            int attempts = getProvisioningAttempts();
            setProvisioningAttempts(attempts + 1);
            Serial.printf("Provisioning attempt %d/%d...\n", attempts + 1, MAX_PROVISIONING_ATTEMPTS);
        }
        else
        {
            // No credentials at all - clear and initialize BLE
            clearWifiPref();
            clearBootPref();
            resetProvisioningAttempts();
        }

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

    currentTask = getTask();
    if (currentTask == "unpair")
    {
        setTask("normal"); // Reset to normal, to avoid loop executions
        printLocalTime();
        delay(1200);
        // NO FUNCTIONALITIES AS OF YET
        Serial.println("\n");
        Serial.println("    ╔═══════════════════════════════════════════════════╗");
        Serial.println("    ║             FIREBASE DATA NODE UNPAIRED           ║");
        Serial.println("    ╚═══════════════════════════════════════════════════╝");
        Serial.println("\n");
    }
    else if (currentTask == "destroy")
    {
        isDestroyed = true;

        firebaseDeleteData();
        firebaseReset();
        setTask("normal"); // Reset to normal, to avoid loop executions
        delay(1200);

        initBLE();
        clearCredentials(); // Resets data on BLE, letting user BLE provision again
        provisioningComplete = false;

        Serial.println("\n");
        Serial.println("    ╔═══════════════════════════════════════════════════╗");
        Serial.println("    ║             FIREBASE DATA NODE REMOVED            ║");
        Serial.println("    ╚═══════════════════════════════════════════════════╝");
        Serial.println("\n");
    }

    // Servo mode: AUTO (default) | MANUAL
    currentCmd = getCommand();
    if (currentCmd == "auto")
    {
        // Serial.printf("\nAUTO: %s\n", currentCmd.c_str());
        currentAngle = -1;
        controlLidAuto(currentMillis);
        // firebaseAddActivitiesData(getLocalTime(), "First Boot", "Smart Dustbin initiated");
    }
    else
    {
        if (currentCmd == "open")
        {
            // This prevents repetitive openLid call
            if (currentAngle != 0)
            {
                currentAngle = 0;
                openLid();
            }
            closeTimestamp = 0;
        }
        else
        {
            if (currentAngle != 90)
            {
                currentAngle = 90;
                closeLid();
                closeTimestamp = millis();
            }

            if (closeTimestamp > 0 && millis() - closeTimestamp >= AUTO_TIMEOUT)
            {
                setCommand("auto");
                currentCmd = "auto";
                Serial.println("\nTimeout reached, switching to AUTO MODE\n");
                firebaseAddActivitiesData(getLocalTime(), "Switch Auto", "switched back to auto-mode");
            }
        }

        // Serial.printf("\nMANUAL: %s\n", currentCmd.c_str());
    }

    if (currentMillis - lastMainLoopMillis >= MAIN_LOOP_INTERVAL)
    {
        if (hasCredentials() && !provisioningComplete)
        {
            // Implement retry logic with exponential backoff
            unsigned long timeSinceLastAttempt = currentMillis - lastProvisioningAttempt;

            if (lastProvisioningAttempt == 0 || timeSinceLastAttempt >= PROVISIONING_RETRY_INTERVAL)
            {
                lastProvisioningAttempt = currentMillis;
                Serial.println("\n    [PROVISIONING] Attempting WiFi connection from main loop...");

                wifiConnected = connectWiFi(getSSID(), getPassword());

                if (wifiConnected)
                {
                    isDestroyed = false;
                    resetProvisioningAttempts(); // Reset counter on success

                    // Don't block here - let Firebase initialize asynchronously
                    Serial.println();
                    Serial.println("    ╔═══════════════════════════════════════════════════╗");
                    Serial.println("    ║           Initializing Firebase (async)...        ║");
                    Serial.println("    ╚═══════════════════════════════════════════════════╝");
                    Serial.println();

                    // Saving to NVS
                    ssid = getSSID();
                    password = getPassword();
                    userUID = getUserUID();
                    binID = getBinID();
                    setWifiPref(ssid, password, userUID, binID);
                    delay(500);

                    savedPrefs = getWifiPref();
                    ssid = savedPrefs[0];
                    password = savedPrefs[1];
                    userUID = savedPrefs[2];
                    binID = savedPrefs[3];

                    provisioningComplete = true;
                    firebaseSetPath(userUID, binID);
                    initFirebase();
                }
                else
                {
                    int attempts = getProvisioningAttempts();
                    if (attempts < MAX_PROVISIONING_ATTEMPTS)
                    {
                        Serial.printf("\n✗ WiFi Connection Failed! (Attempt %d/%d)\n", attempts + 1, MAX_PROVISIONING_ATTEMPTS);
                        Serial.println("Will retry in 30 seconds...");
                    }
                    else
                    {
                        Serial.println("\n✗ Max provisioning attempts reached. Entering BLE provisioning mode.");
                    }
                }
            }
        }

        // Checks to see if firebase has already initialized
        if (wifiConnected && firebaseReady() && !initFirebaseRTDB)
        {
            String *firebaseData = getfirebaseData();
            String name = firebaseData[0];
            String location = firebaseData[1];

            // This ensures existing dustbin's data will not be overwritten
            if (!(firebaseData[0] == "null" || firebaseData[1] == "null"))
            {
                Serial.println("No need for initial data");
                firebaseUpdateData(name, location);
                firebaseSubscribe();
                initFirebaseRTDB = true;
                return;
            }

            firebaseSetData("", "");
            firebaseSubscribe();
            firebaseAddActivitiesData(getLocalTime(), "First Boot", "Smart Dustbin initiated");
            initFirebaseRTDB = true;
        }

        if (wifiConnected && (millis() - lastWiFiCheck > WIFI_CHECK_INTERVAL))
        {
            lastWiFiCheck = millis();

            if (!isWifiConnected() && initFirebaseRTDB)
            {
                Serial.println("WiFi connection lost! Reconnecting...");
                wifiConnected = connectWiFi(ssid, password);
                while (WiFi.status() != WL_CONNECTED)
                    delay(500);
            }
        }

        if (wifiConnected)
        {
            if (isDestroyed)
                return;

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
