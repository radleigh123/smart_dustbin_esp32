#include "config.h"
#include "ultrasonic_sensor.h"
#include <Arduino.h>

static unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 10000; // 5 minutes in milliseconds
static float lastDistance = 0;

float measureDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    float duration = pulseIn(ECHO_PIN, HIGH);
    return (duration * 0.0343) / 2;
}

void initUltrasonicSensor()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    lastDistance = measureDistance(); // Initial reading
}

float getDistance(unsigned long currentMillis)
{
    if (currentMillis - lastReadTime >= READ_INTERVAL)
    {
        lastDistance = measureDistance();
        lastReadTime = currentMillis;
        Serial.print("Ultrasonic Sensor Distance: ");
        Serial.println(lastDistance);
    }
    return lastDistance;
}
