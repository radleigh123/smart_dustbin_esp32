#include "firebase_app.h"
#include <FirebaseClient.h>

UserAuth user_auth(WEB_API_KEY, USER_EMAIL, USER_PASS);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient client(ssl_client);
RealtimeDatabase Database;

String binId = "trash_bins/";

void initFirebase()
{
    // Configure SSL client
    ssl_client.setInsecure(); // NOTE: Use with caution - accepts any certificate
    ssl_client.setTimeout(15000);
    ssl_client.setHandshakeTimeout(15000);

    // Firebase init
    initializeApp(client, app, getAuth(user_auth), processData, "🔐 authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(FIREBASE_HOST);
    delay(1000);

    // Listen for realtime changes, continue to firebaseSetPath, subscribe
    Database.setSSEFilters();
}

void firebaseLoop()
{
    app.loop();
}

bool firebaseReady()
{
    return app.ready();
}

/**
 * Set initial data in Firebase Realtime Database
 * Status<int>: 0 - Empty, 1 - Normal, 2 - Full
 * Name<string>: Bin name
 * Location<string>: Bin location
 * Fill Level<int>: Percentage of fill level
 */
void firebaseSetData(const String &binName, const String &binLocation)
{
    const String name = binName.length() > 0 ? binName : "Smart Dustbin 01";
    const String location = binLocation.length() > 0 ? binLocation : "Building A - Lobby";

    Database.set<String>(client, binId + "status", "auto", processData, "INIT_STATUS");
    Database.set<String>(client, binId + "command", "open", processData, "INIT_COMMAND");
    Database.set<String>(client, binId + "name", name, processData, "INIT_BIN_NAME");
    Database.set<String>(client, binId + "location", location, processData, "INIT_BIN_LOCATION");
    Database.set<int>(client, binId + "fillLevel", 0, processData, "INIT_FILL_LEVEL");
    Database.set<int>(client, binId + "distance", 0, processData, "INIT_DISTANCE");
}

/**
 * Calculate fill_level based on distance and update in Firebase
 */
void firebaseUpdateFillLevel(float distance)
{
    int fillLevel = 0;
    int maxDistance = 25; // In cm
    if (distance >= maxDistance)
    {
        fillLevel = 0; // Empty
    }
    else if (distance <= 0.0)
    {
        fillLevel = 100; // Full
    }
    else
    {
        fillLevel = static_cast<int>(((maxDistance - distance) / maxDistance) * 100);
    }

    Database.set<int>(client, binId + "fillLevel", fillLevel, processData, "SET_FILL_LEVEL");
}

void firebaseUpdateUltrasonicData(float distance)
{
    int data = static_cast<int>(distance);
    Database.set<int>(client, binId + "distance", data, processData, "SET_DISTANCE");
}

void firebaseSetPath(const String &id)
{
    binId = "trash_bins/" + id + "/";

    Serial.println("Subscribing to RTDB...");
    Database.get(client, binId, processData, true, "SUB_STATUS");
}

void processData(AsyncResult &result)
{
    if (!result.isResult())
        return;

    if (result.isEvent())
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", result.uid().c_str(), result.eventLog().message().c_str(), result.eventLog().code());

    if (result.isDebug())
        Firebase.printf("Debug task: %s, msg: %s\n", result.uid().c_str(), result.debug().c_str());

    if (result.isError())
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", result.uid().c_str(), result.error().message().c_str(), result.error().code());

    if (!result.available())
        return;

    Firebase.printf("task: %s, payload: %s\n", result.uid().c_str(), result.c_str());
    String payload = result.c_str();
    String path = result.path();

    Serial.printf("Path: %s\nPayload: %s\n", path.c_str(), payload.c_str());
}