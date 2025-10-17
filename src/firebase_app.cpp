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