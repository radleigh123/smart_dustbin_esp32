#include <ESP32Servo.h>
#include <Arduino.h>
#include "config.h"
#include "servo_controller.h"
#include "ir_sensor.h"

static Servo myServo;

static const int OPEN_ANGLE = 0;
static const int CLOSE_ANGLE = 90;

static bool isLidOpen = false;
static unsigned long lidOpenTime = 0;

const unsigned long LID_OPEN_DURATION = 3000; // Time to keep lid open in ms

bool isLidCurrentlyOpen()
{
    return isLidOpen;
}

void initServo()
{
    myServo.setPeriodHertz(50);
    myServo.attach(SERVO_PIN, 500, 2400);
    closeLid();
}

void openLid()
{
    for (int pos = 90; pos >= OPEN_ANGLE; pos -= 1)
    {
        myServo.write(pos);
        delay(5);
    }
    // myServo.write(OPEN_ANGLE);
    isLidOpen = true;
    lidOpenTime = millis();
    digitalWrite(LED_PIN_GREEN, HIGH);
    digitalWrite(LED_PIN_RED, LOW);
    // Serial.println("Lid opened");
}

void closeLid()
{
    for (int pos = 0; pos <= CLOSE_ANGLE; pos += 1)
    {
        myServo.write(pos);
        delay(10);
    }
    // myServo.write(CLOSE_ANGLE);
    isLidOpen = false;
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_RED, HIGH);
    // Serial.println("Lid closed");
}

void controlServo(float distance)
{
    int angle = map(distance, 0, 100, 0, 180);
    myServo.write(angle);
}

void controlLidAuto(unsigned long currentMillis) {
    bool detected = isObjectDetected();

    if (detected && !isLidOpen)
    {
        openLid();
    }

    if (!detected && isLidOpen && (currentMillis - lidOpenTime >= LID_OPEN_DURATION))
    {
        closeLid();
    }
}
