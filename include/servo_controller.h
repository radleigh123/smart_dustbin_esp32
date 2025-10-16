#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <ESP32Servo.h>

void initServo();
void controlServo(float distance);
void controlLidAuto(unsigned long currentMillis);

#endif