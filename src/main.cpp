#include <Arduino.h>
#include "config.h"
#include "servo_controller.h"
#include "ultrasonic_sensor.h"
#include "wifi_manager.h"

void setup()
{
    Serial.begin(115200);

    initWifiProvisioning();
    initServo();
}

void loop()
{
    if (isWifiConnected())
    {
        float distance = getDistance();
        controlServo(distance);
    }
}