#ifndef CONFIG_H
#define CONFIG_H

/*
╔════════════════════════════════════════╗
║            Pin Configuration           ║
╠════════════════════════════════════════╣
║ Servo Pin        : GPIO 26             ║
║ -------------------------------------- ║
║ Ultrasonic Trig  : GPIO 22             ║
║ Ultrasonic Echo  : GPIO 23             ║
║ -------------------------------------- ║
║ IR Sensor Pin    : GPIO 27             ║
╚════════════════════════════════════════╝*/
// Servo
#define SERVO_PIN 26

// Ultrasonic Sensor
#define TRIG_PIN 22
#define ECHO_PIN 23

// IR Sensor
#define IR_SENSOR_PIN 27

#define DISTANCE_THRESHOLD 10

// Firebase Credentials
#define WEB_API_KEY "AIzaSyAjVoD7YONx-VOaE_Ls8g8difj7a7_xlXI"
#define FIREBASE_HOST "https://smart-dustbin-43323-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "admin@admin.com"
#define USER_PASS "123456"

#endif