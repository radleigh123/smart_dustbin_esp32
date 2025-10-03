#include <ESP32Servo.h>
#include "config.h"
#include "servo_controller.h"

Servo myServo;

void initServo()
{
    myServo.setPeriodHertz(50);
    myServo.attach(SERVO_PIN, 500, 2400);
}

void controlServo(float distance)
{
    int angle = map(distance, 0, 100, 0, 180);
    myServo.write(angle);
}