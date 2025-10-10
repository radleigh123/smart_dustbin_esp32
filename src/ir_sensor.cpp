#include "config.h"
#include "ir_sensor.h"
#include <Arduino.h>

void initIRSensor()
{
    pinMode(IR_SENSOR_PIN, INPUT);
}

bool isObjectDetected()
{
    int sensorValue = digitalRead(IR_SENSOR_PIN);
    return (sensorValue == LOW); // LOW means object detected
}