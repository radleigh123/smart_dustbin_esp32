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
    Database.setSSEFilters();
    Database.get(client, "/test/data", processData, true, "🧾 RTDB_GET_TASK");
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

    Database.set<int>(client, binId + "status", 0, processData, "📝 RTDB_SET_STATUS_TASK");
    Database.set<String>(client, binId + "name", name, processData, "📝 RTDB_SET_NAME_TASK");
    Database.set<String>(client, binId + "location", location, processData, "📝 RTDB_SET_LOCATION_TASK");
    Database.set<int>(client, binId + "fillLevel", 0, processData, "📝 RTDB_SET_FILL_LEVEL_TASK");
    Database.set<int>(client, binId + "distance", 0, processData, "📝 RTDB_SET_DISTANCE_TASK");
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
    /* if (distance >= 35)
    {
        fillLevel = 0; // Empty
    }
    else if (distance <= 0.0)
    {
        fillLevel = 100; // Full
    }
    else
    {
        fillLevel = static_cast<int>(((100.0 - distance) / 100.0) * 100);
    } */

    Database.set<int>(client, binId + "fillLevel", fillLevel, processData, "📝 RTDB_SET_FILL_LEVEL_TASK");
}

void firebaseUpdateUltrasonicData(float distance)
{
    int data = static_cast<int>(distance);
    Database.set<int>(client, binId + "distance", data, processData, "📝 RTDB_SET_DISTANCE_TASK");
}

void firebaseSetPath(const String &id)
{
    binId = "trash_bins/" + id + "/";
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

    if (result.available())
        Firebase.printf("task: %s, payload: %s\n", result.uid().c_str(), result.c_str());
}