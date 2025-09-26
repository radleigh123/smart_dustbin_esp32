#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#define TRIG_PIN 22
#define ECHO_PIN 23
#define SERVO_PIN 26
#define DISTANCE_THRESHOLD 10 // cm

#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

// Wi-Fi credentials
#define WIFI_SSID "GAMSUNG"
#define WIFI_PASSWORD "aleahgwapa"

// Firebase project credentials
#define WEB_API_KEY "AIzaSyAjVoD7YONx-VOaE_Ls8g8difj7a7_xlXI"
#define FIREBASE_HOST "https://smart-dustbin-43323-default-rtdb.asia-southeast1.firebasedatabase.app/" // For RTDB
#define USER_EMAIL "admin@admin.com"
#define USER_PASS "123456"

// Authentication
UserAuth user_auth(WEB_API_KEY, USER_EMAIL, USER_PASS);

// Firebase Components
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;

Servo servo;

// Variables for ultrasonic sensor
float filterArray[20]; // array to hold the samples
float distance;

// Variables for database
int status = 0;           // 0: CLOSE, 1: OPEN (from ultrasonic)
bool manualMode = false;  // for ADMIN ONLY
int manualServoAngle = 0; // for ADMIN ONLY

String binId;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // Send data every 10 seconds

// User function
void processData(AsyncResult &aResult);

// Ultrasonic sensor function
float ultrasonicMeasure();
double getUltrasonicDistance();

// Firebase setup
void connectToWiFi();

// Utility function
String getTrashBinPath(const String &binId);

void setup()
{
    Serial.begin(115200);

    binId = WiFi.macAddress();
    binId.replace(":", "");
    Serial.println("Bin ID: " + binId);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    servo.attach(SERVO_PIN, 500, 2400); // min and max in microseconds
    servo.write(0);                     // initial position

    connectToWiFi(); // PURPOSE: Communicate to Firebase
}

void loop()
{
    /* app.loop(); // Required for Firebase to process authentication and tasks

    // Check if Firebase is authenticated and ready
    if (!app.ready())
        return; */

    // === LOCAL SENSOR LOGIC ===
    distance = getUltrasonicDistance();
    if (distance < DISTANCE_THRESHOLD)
    {
        status = 1; // 1: OPEN
        servo.write(90);
    }
    else
    {
        status = 0; // 0: CLOSE
        servo.write(0);
    }

    Serial.printf("Distance: %.2f cm, Status: %s\n", distance, status == 1 ? "OPEN" : "CLOSE");
}

void processData(AsyncResult &aResult)
{
    if (!aResult.isResult())
        return;

    if (aResult.isEvent())
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

    if (aResult.isDebug())
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

    if (aResult.isError())
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

    if (aResult.available())
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}

float ultrasonicMeasure()
{
    // generate 10-microsecond pulse to TRIG pin
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    // measure duration of pulse from ECHO pin
    float duration_us = pulseIn(ECHO_PIN, HIGH);
    return 0.017 * duration_us;
}

double getUltrasonicDistance()
{
    // 1. TAKING MULTIPLE MEASUREMENTS AND STORE IN AN ARRAY
    for (int sample = 0; sample < 20; sample++)
    {
        filterArray[sample] = ultrasonicMeasure();
        delay(30); // to avoid untrasonic interfering
    }

    // 2. SORTING THE ARRAY IN ASCENDING ORDER
    for (int i = 0; i < 19; i++)
    {
        for (int j = i + 1; j < 20; j++)
        {
            if (filterArray[i] > filterArray[j])
            {
                float swap = filterArray[i];
                filterArray[i] = filterArray[j];
                filterArray[j] = swap;
            }
        }
    }

    // 3. FILTERING NOISE
    // + the five smallest samples are considered as noise -> ignore it
    // + the five biggest  samples are considered as noise -> ignore it
    // ----------------------------------------------------------------
    // => get average of the 10 middle samples (from 5th to 14th)
    double sum = 0;
    for (int sample = 5; sample < 15; sample++)
    {
        sum += filterArray[sample];
    }

    return sum / 10;
}

void connectToWiFi()
{
    /* String ssid = prefs.getString("ssid", "");
    String password = prefs.getString("password", "");
    prefs.end();

    if (WIFI_SSID == "" || WIFI_PASSWORD == "")
    {
        Serial.println("No Wi-Fi credentials stored");
        return;
    } */

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("Connecting to Wi-Fi SSID: %s\n", WIFI_SSID);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 40)
    {
        delay(500);
        Serial.print(".");
        retries++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected to Wi-Fi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        // Configure SSL client
        ssl_client.setInsecure(); // Use with caution, for testing only
        ssl_client.setTimeout(15000);
        ssl_client.setHandshakeTimeout(2000);

        // Initialize Firebase
        initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
        app.getApp<RealtimeDatabase>(Database);
        Database.url(FIREBASE_HOST);

        delay(1000);

        // Subscribe to remote status changes
        // Database.get(aClient, getTrashBinPath(binId), processData, "RTDB_Listen_Status");
        // listen to any changes under /test/data
        Database.setSSEFilters();
        Database.get(aClient, "/test/data", processData, true, "RTDB_Listen_Status");
    }
    else
    {
        Serial.println("Wi-Fi failed to connect");
    }
}

String getTrashBinPath(const String &binId)
{
    return "/trash_bins/" + binId + "/status";
}
