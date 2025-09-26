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

Servo servo;

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

// Variables for ultrasonic sensor
float filterArray[20]; // array to hold the samples
float distance;

// Variables for database
int localStatus = 0;  // 0: CLOSE, 1: OPEN (from ultrasonic)
int remoteStatus = 0; // 0: CLOSE, 1: OPEN (from Firebase)
int finalStatus = 0;  // what the servo actually does

String binId = "bin1";
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // Send data every 10 seconds

// User function
void processData(AsyncResult &aResult);

// Ultrasonic sensor function
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

// Ultrasonic sensor function with noise filtering
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

String getTrashBinPath(const String &binId)
{
    return "/trash_bins/" + binId + "/status";
}

void setup()
{
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    servo.attach(SERVO_PIN, 500, 2400); // min and max in microseconds
    servo.write(0);                     // initial position

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

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

void loop()
{
    app.loop(); // Required for Firebase to process authentication and tasks

    // Check if Firebase is authenticated and ready
    if (!app.ready())
        return;

    // === LOCAL SENSOR LOGIC ===
    distance = getUltrasonicDistance();
    if (distance < DISTANCE_THRESHOLD)
    {
        localStatus = 1; // 1: OPEN
    }
    else
    {
        localStatus = 0; // 0: CLOSE
    }

    // === FINAL STATUS (remote overrides local) ===
    // Rule: If remote says OPEN → force open
    // Otherwise use ultrasonic
    if (remoteStatus == 1)
    {
        finalStatus = 1;
    }
    else
    {
        finalStatus = localStatus;
    }

    // === Actuate servo ===
    if (finalStatus == 1)
    {
        servo.write(90);
    }
    else
    {
        servo.write(0);
    }

    // === Update DB if LOCAL changed ===
    static int lastLocalStatus = -1;
    if (localStatus != lastLocalStatus && millis() - lastSendTime > sendInterval)
    {
        lastLocalStatus = localStatus;
        lastSendTime = millis();
        // Database.set<int>(aClient, getTrashBinPath(binId), localStatus, processData, "RTDB_Send_Status");
        Database.set<int>(aClient, "/test/data", localStatus, processData, "RTDB_Update_Status");
        Serial.printf("Updated DB with local status: %s\n", localStatus ? "OPEN" : "CLOSE");
    }

    Serial.printf("Distance: %.2f cm | Local: %d | Remote: %d | Final: %d\n",
                  distance, localStatus, remoteStatus, finalStatus);
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
    {
        String uid = aResult.uid();
        String payload = aResult.c_str();

        Serial.println("=== Firebase Event ===");
        Serial.printf("Task: %s\n", uid.c_str());
        Serial.printf("Raw payload: %s\n", payload.c_str());

        // Only process streaming events from the listener
        if (uid == "RTDB_Listen_Status")
        {
            // Firebase can send various formats depending on the event type
            // Let's try multiple parsing approaches

            // Try parsing as {"data": value} format
            if (payload.indexOf("\"data\"") >= 0)
            {
                int valueStart = payload.indexOf(":", payload.indexOf("\"data\"")) + 1;
                int valueEnd = payload.indexOf("}", valueStart);
                String valueStr = payload.substring(valueStart, valueEnd);
                valueStr.trim();

                remoteStatus = valueStr.toInt();
                Serial.printf("Parsed remote status (format 1): %d\n", remoteStatus);
            }
            // Try parsing as {"path": "/", "data": value} format
            else if (payload.indexOf("\"path\"") >= 0 && payload.indexOf("\"data\"") >= 0)
            {
                int valueStart = payload.indexOf(":", payload.indexOf("\"data\"")) + 1;
                int valueEnd = payload.indexOf("}", valueStart);
                if (valueEnd == -1)
                    valueEnd = payload.indexOf(",", valueStart);
                if (valueEnd == -1)
                    valueEnd = payload.length() - 1;

                String valueStr = payload.substring(valueStart, valueEnd);
                valueStr.trim();

                remoteStatus = valueStr.toInt();
                Serial.printf("Parsed remote status (format 2): %d\n", remoteStatus);
            }
            // If simple numeric value
            else if (payload.length() < 10 && isDigit(payload[0]))
            {
                remoteStatus = payload.toInt();
                Serial.printf("Parsed remote status (format 3): %d\n", remoteStatus);
            }
        }
    }
}