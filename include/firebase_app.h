#ifndef FIREBASE_APP_H
#define FIREBASE_APP_H

#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include "config.h"

void initFirebase(const String &path);
void firebaseLoop();
bool firebaseReady();
void firebaseSubscribe();
void firebaseSetData(const String &binName, const String &binLocation);
void firebaseSetPath(const String &id);
String firebaseGetPath();
void firebaseSubscribe();
String *getfirebaseData();
void firebaseUpdateUltrasonicData(float distance);
void processData(AsyncResult &result);

String getCommand();

#endif // FIREBASE_APP_H