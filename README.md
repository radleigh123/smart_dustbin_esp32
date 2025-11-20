# Smart Dustbin -- ESP32 Firmware

A simple IoT-based smart trash bin powered by ESP32. The firmware controls sensors and actuators, syncs data to cloud (Firebase), and enables real-time monitoring via the companion Android app.

<details open>

<summary>Technology Stack</summary>

|    Component    | Description                           |
|:---------------:| ------------------------------------- |
| Microcontroller | ESP32 (PlatformIO)                   |
|     Sensor      | Ultrasonic (HC-SR04) / IR sensor     | 
|    Actuator     | Servo motor                           |
| Cloud Platform  | Firebase (Realtime DB, FCM) |

</details>

## Transmission between ESP32, client, and cloud

```mermaid
sequenceDiagram
    participant Bin as Smart Dustbin (ESP32)
    participant Cloud as Firebase RTDB
    participant App as Client (Android)

    loop Connection
    Bin->>Cloud: Send sensor data (distance, fillLevel, name, location)
    Cloud-->>App: Sync bin data in real-time
    App->>Cloud: Request list of bins for user
    Cloud-->>App: Return bins with commands

    App->>Cloud: Update bin command (Open / Close / Auto)
    Cloud-->>Bin: Push command update
    Bin->>Bin: Execute command (servo motor action)

    Note over Bin,Cloud: Continuous sensor updates
    Note over App,Cloud: Real-time monitoring and control
    end
```

## Related Projects

- Companion Android app: [Smart Dustbin -- Android App](https://github.com/radleigh123/smart-dustbin)
