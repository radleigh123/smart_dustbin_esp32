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
void firebaseSetData(const String &binName, const String &binLocation);
void firebaseUpdateFillLevel(float distance);
void firebaseUpdateUltrasonicData(float distance);
void firebaseSetPath(const String &id);
void processData(AsyncResult &result);

String getCachedMode();
String getCachedCommand();

#endif // FIREBASE_APP_H