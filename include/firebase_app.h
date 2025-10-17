#ifndef FIREBASE_APP_H
#define FIREBASE_APP_H

#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include "config.h"

void initFirebase();
void firebaseLoop();
bool firebaseReady();
void firebaseUpdateUltrasonicData(float distance);
void firebaseSetPath(const String &id);
void processData(AsyncResult &result);

#endif // FIREBASE_APP_H