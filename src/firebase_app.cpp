#include "firebase_app.h"
#include <FirebaseClient.h>

WiFiClientSecure ssl_client, stream_ssl_client;
using AsyncClient = AsyncClientClass;
// AsyncClient client(ssl_client), streamClientStatus(stream_ssl_client_status), streamClientCommand(stream_ssl_client_command);
AsyncClient client(ssl_client), streamClient(stream_ssl_client);

UserAuth user_auth(WEB_API_KEY, USER_EMAIL, USER_PASS);
FirebaseApp app;
RealtimeDatabase Database;

String cachedCommand = "auto";

String binId = "trash_bins/";

void initFirebase(const String &path)
{
    // Configure SSL client
    ssl_client.setInsecure(); // NOTE: Use with caution - accepts any certificate
    ssl_client.setTimeout(15000);
    ssl_client.setHandshakeTimeout(5);

    stream_ssl_client.setInsecure(); // NOTE: Use with caution - accepts any certificate
    stream_ssl_client.setTimeout(15000);
    stream_ssl_client.setHandshakeTimeout(5);

    // Firebase init
    initializeApp(client, app, getAuth(user_auth), processData, "🔐 authTask");
    delay(1200);
    app.getApp<RealtimeDatabase>(Database);
    Database.url(FIREBASE_HOST);

    // Listen for realtime changes, continue to firebaseSetPath, subscribe
    streamClient.setSSEFilters("get,put");
    Database.get(streamClient, path + "commands", processData, true, "SUB_COMMANDS");
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
 * name<String>: Bin name
 * location<String>: Bin location
 * fillLevel<int>: Percentage of fill level
 * distance<int>: Percentage of fill level
 * mode<String>: auto | manual
 * command<String>: auto | open | close
 */
void firebaseSetData(const String &binName, const String &binLocation)
{
    // const String name = binName.length() > 0 ? binName : "Smart Dustbin 01";
    // const String location = binLocation.length() > 0 ? binLocation : "Building A - Lobby";
    const String cmd = "auto";

    // Database.set<String>(client, binId + "name", name, processData, "INIT_BIN_NAME");
    // Database.set<String>(client, binId + "location", location, processData, "INIT_BIN_LOCATION");
    // Database.set<int>(client, binId + "fillLevel", 0, processData, "INIT_FILL_LEVEL");
    // Database.set<int>(client, binId + "distance", 0, processData, "INIT_DISTANCE");
    // Database.set<String>(client, binId + "/commands/mode", "auto", processData, "INIT_MODE");
    Database.update<object_t>(client, binId + "commands", object_t("{\"command\":\"auto\"}"), processData, "DEFAULT_UPDATE_MODE");
}

String firebaseGetPath()
{
    return binId;
}

void firebaseSetPath(const String &id)
{
    binId = "trash_bins/" + id + "/";
    Serial.println("----------------------");
    Serial.printf("SETPATH=%s\n", binId.c_str());
    Serial.println("----------------------");
}

/**
 * Calculate fill_level based on distance and update in Firebase
 */
void firebaseUpdateUltrasonicData(float distance)
{
    JsonWriter writer;
    object_t json, obj1, obj2;

    // distance
    int data = static_cast<int>(distance);

    // fill level
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

    writer.create(obj1, "distance", data);
    writer.create(obj2, "fillLevel", fillLevel);
    writer.join(json, 2, obj1, obj2);

    Database.update<object_t>(client, binId, json, processData, "SET_DISTANCE_AND_FILL");
    // Database.set<int>(client, binId + "fillLevel", fillLevel, processData, "SET_FILL_LEVEL");
    // Database.set<int>(client, binId + "distance", data, processData, "SET_DISTANCE");
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

    RealtimeDatabaseResult &stream = result.to<RealtimeDatabaseResult>();

    if (stream.isStream())
    {
        const String data = stream.to<const char *>();

        if (data == "auto" || data == "open" || data == "close")
            cachedCommand = data;

        Serial.println("========================================");
        Firebase.printf("task: %s\n", result.uid().c_str());
        Firebase.printf("event: %s\n", stream.event().c_str());
        Firebase.printf("path: %s\n", stream.dataPath().c_str());
        Firebase.printf("data: %s\n", data.c_str());
    }
    else
    {
        Serial.println("========================================");
        // Serial.printf("path:%s\n", result.path().c_str());
        Serial.println("path: " + binId);
        Firebase.printf("task: %s, payload: %s\n", result.uid().c_str(), result.c_str());
    }
}

String getCommand()
{
    return cachedCommand;
}