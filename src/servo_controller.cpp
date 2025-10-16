#include <ESP32Servo.h>
#include "config.h"
#include "servo_controller.h"
#include "ir_sensor.h"

Servo myServo;

#define EXE_INTERVAL_1 1000
#define EXE_INTERVAL_2 3000
unsigned long lastExecuteMillis1 = 0;
unsigned long lastExecuteMillis2 = 0;

static bool isLidOpen = false;
static unsigned long lidOpenTime = 0;
const unsigned long LID_OPEN_DURATION = 3000; // Time to keep lid open in ms

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

void controlLidAuto(unsigned long currentMillis) {
    if (isObjectDetected()) {
        // Object detected - open lid if not already open
        if (!isLidOpen) {
            int angle = map(90, 0, 100, 0, 180);
            controlServo(angle);
            isLidOpen = true;
            lidOpenTime = currentMillis;
            Serial.println("Opening lid");
        }
    } else if (isLidOpen && (currentMillis - lidOpenTime >= LID_OPEN_DURATION)) {
        // No object detected and lid has been open for duration - close it
        int angle = map(25, 0, 100, 0, 180);
        controlServo(angle);
        isLidOpen = false;
        Serial.println("Closing lid");
    }
}
