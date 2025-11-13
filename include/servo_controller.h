#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <ESP32Servo.h>

void initServo();
void openLid();
void closeLid();
void controlServo(float distance);
void controlLidAuto(unsigned long currentMillis);
bool isLidCurrentlyOpen();

#endif