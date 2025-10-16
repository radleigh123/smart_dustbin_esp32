#include <ESP32Servo.h>
#include "config.h"
#include "servo_controller.h"

Servo myServo;
int currentAngle = 45;
unsigned long lastMoveTime = 0;
const int MOVE_INTERVAL = 30; // Lower = faster, Higher = slower

void initServo()
{
    myServo.setPeriodHertz(50);
    myServo.attach(SERVO_PIN, 500, 2400);
    // myServo.write(currentAngle);
}

void controlServo(float distance)
{
    int angle = map(distance, 0, 100, 0, 180);
    myServo.write(angle);
    /* int targetAngle = map(distance, 0, 100, 0, 180);

    if (millis() - lastMoveTime >= MOVE_INTERVAL)
    {
        if (currentAngle < targetAngle)
        {
            currentAngle++;
        }
        else if (currentAngle > targetAngle)
        {
            currentAngle--;
        }
        myServo.write(currentAngle);
        lastMoveTime = millis();
    } */
}