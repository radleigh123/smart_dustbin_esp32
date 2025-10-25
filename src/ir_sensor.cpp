#include "config.h"
#include "ir_sensor.h"
#include <Arduino.h>

void initIRSensor()
{
    pinMode(IR_SENSOR_PIN, INPUT);
    pinMode(LED_PIN_GREEN, OUTPUT);
    pinMode(LED_PIN_RED, OUTPUT);
}

bool isObjectDetected()
{
    int sensorValue = digitalRead(IR_SENSOR_PIN);
    bool objectDetected = (sensorValue == LOW);
    static unsigned long detectionStartTime = 0;
    static bool wasDetected = false;
    if (objectDetected)
    {
        if (!wasDetected)
        {
            detectionStartTime = millis();
            wasDetected = true;
        }
        else if (millis() - detectionStartTime >= 300)
        {
            return true;
        }
    }
    else
    {
        wasDetected = false;
    }
    return false;
}