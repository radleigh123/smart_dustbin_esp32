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
void firebaseSubscribe();
void firebaseReset();

String firebaseGetPath();
void firebaseSetPath(const String &userId, const String &binId);
String *getfirebaseData();
void firebaseSetData(const String &binName, const String &binLocation);
void firebaseUpdateData(const String &binName, const String &binLocation);
void firebaseDeleteData();

void firebaseUpdateUltrasonicData(float distance);

void firebaseAddActivitiesData(const String timestamp, const String title, String message);

void processData(AsyncResult &result);

String getCommand();
void setCommand(String cmd);
String getTask();
void setTask(String task);

#endif // FIREBASE_APP_H