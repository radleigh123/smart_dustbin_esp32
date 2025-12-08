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
String cachedTask = "normal";

String binId = "trash_bins/";

static bool isReset = false;

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
    streamClient.setSSEFilters("get,put,patch");
    delay(1200);
    // Database.get(streamClient, path + "commands", processData, true, "SUB_COMMANDS");
    // Database.get(streamClient, binId + "commands", processData, true, "SUB_COMMANDS");
}

void firebaseLoop()
{
    app.loop();
}

bool firebaseReady()
{
    return app.ready();
}

void firebaseSubscribe()
{
    Database.get(streamClient, binId + "commands", processData, true, "SUB_COMMANDS");
}

void firebaseReset()
{
    isReset = true;
    Database.resetApp();
}

String firebaseGetPath()
{
    return binId;
}

void firebaseSetPath(const String &id)
{
    binId = "trash_bins/" + id + "/";
    Serial.println("========================================");
    Serial.printf("SETPATH = %s\n", binId.c_str());
    Serial.println("========================================");
}

String *getfirebaseData()
{
    static String data[4];
    data[0] = Database.get<String>(client, binId + "name");
    data[1] = Database.get<String>(client, binId + "location");

    Serial.println("Firebase data retrieved:");
    Serial.printf("Name: %s\n", data[0].c_str());
    Serial.printf("Location: %s\n", data[1].c_str());

    return data;
}

/**
 * Set initial data in Firebase Realtime Database
 * name<String>: Bin name
 * location<String>: Bin location
 * fillLevel<int>: Percentage of fill level
 * distance<int>: Percentage of fill level
 * mode<String>: auto | manual
 * command<String>: auto | open | close
 * task<String>: normal | unpair | destroy
 */
void firebaseSetData(const String &binName, const String &binLocation)
{
    Serial.println("========================================");
    Serial.println("Setting initial dustbin data...");
    Serial.println("path: " + binId);

    JsonWriter writer;
    object_t json, name_n, location_n, fillLevel_n, distance_n;
    object_t commands_n, cmd_n, mode_n, task_n;

    const String name = binName.length() > 0 ? binName : "Smart Dustbin Name";
    const String location = binLocation.length() > 0 ? binLocation : "Location Name";
    const int fillLevel = 0;
    const int distance = 0;
    const String cmd = "auto";
    const String mode = "auto";
    const String task = "normal";

    // Top-level fields
    writer.create(name_n, "name", name);
    writer.create(location_n, "location", location);
    writer.create(fillLevel_n, "fillLevel", fillLevel);
    writer.create(distance_n, "distance", distance);

    // Nested commands object
    writer.create(cmd_n, "command", cmd);
    writer.create(mode_n, "mode", mode);
    writer.create(task_n, "task", task);
    writer.join(commands_n, 3, cmd_n, mode_n, task_n); // { "command":"auto","mode":"auto","task":"normal" }
    writer.create(commands_n, "commands", commands_n); // { "commands": { "command":"auto","mode":"auto","task":"normal" } }

    // Final join
    writer.join(json, 5, name_n, location_n, fillLevel_n, distance_n, commands_n);
    Database.set<object_t>(client, binId, json, processData, "DEFAULT_UPDATE_MODE");
    Serial.println("========================================");
}

void firebaseDeleteData()
{
    Database.remove(client, binId, processData, "DELETE_DATA_TASK");
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

        // if (data == "auto" || data == "open" || data == "close")
        //     cachedCommand = data;
        if (data == "{\"command\":\"open\",\"mode\":\"manual\",\"task\":\"normal\"}")
            cachedCommand = "open";
        else if (data == "{\"command\":\"close\",\"mode\":\"manual\",\"task\":\"normal\"}")
            cachedCommand = "close";
        else
            cachedCommand = "auto";

        Serial.printf("\ncached command: %s\n", cachedCommand.c_str());

        if (data == "normal" || data == "unpair" || data == "destroy")
        {
            cachedTask = data;
        }

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

void setCommand(String cmd)
{
    cachedCommand = cmd;
    String mode = cmd != "auto" ? "manual" : "auto";

    JsonWriter writer;
    object_t json, cmd_n, mode_n;

    writer.create(cmd_n, "command", cmd);
    writer.create(mode_n, "mode", mode);
    writer.join(json, 2, cmd_n, mode_n);

    Database.update<object_t>(client, binId + "commands", json, processData, "UPDATE_CMD_TASK");
}

String getTask()
{
    return cachedTask;
}

void setTask(String task)
{
    cachedTask = task;

    if (isReset)
        return;

    JsonWriter writer;
    object_t json, task_n;

    writer.create(task_n, "task", task);
    writer.join(json, 1, task_n);

    Database.update<object_t>(client, binId + "commands", json, processData, "UPDATE_CMD_TASK");
}