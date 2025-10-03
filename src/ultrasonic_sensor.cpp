#include "config.h"
#include "ultrasonic_sensor.h"
#include <Arduino.h>

void initUltrasonicSensor()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

float getDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    float duration = pulseIn(ECHO_PIN, HIGH);
    return (duration * 0.0343) / 2;
}
