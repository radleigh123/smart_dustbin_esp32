# BLE Provisioning for Smart Dustbin ESP32

## Overview
This implementation provides BLE (Bluetooth Low Energy) provisioning for your Smart Dustbin ESP32 device. You can use the NRF Connect mobile app to configure WiFi credentials without hardcoding them in your code.

## Features
- ✅ BLE-based WiFi provisioning
- ✅ Compatible with NRF Connect app (Android/iOS)
- ✅ Real-time status notifications
- ✅ Automatic WiFi connection
- ✅ Connection monitoring
- ✅ Retry mechanism on failure

## Setup Instructions

### 1. Hardware Required
- ESP32 Development Board
- USB Cable for programming
- Smartphone with NRF Connect app

### 2. Software Required
- PlatformIO (already configured)
- NRF Connect app:
  - **Android**: [Google Play Store](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)
  - **iOS**: [App Store](https://apps.apple.com/app/nrf-connect/id1054362403)

### 3. Upload the Code
```bash
pio run --target upload
pio device monitor
```

## How to Use with NRF Connect

### Step 1: Open NRF Connect App
1. Launch the NRF Connect app on your smartphone
2. Make sure Bluetooth is enabled

### Step 2: Scan for Devices
1. Tap the **SCAN** button
2. Look for device named: **SmartDustbin_ESP32**
3. Tap **CONNECT** next to the device

### Step 3: Discover Services
1. After connecting, tap **"Discover services"** or wait for auto-discovery
2. Look for the service with UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
3. Expand the service to see the characteristics

### Step 4: Write WiFi Credentials

#### Write SSID:
1. Find characteristic with UUID: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
2. Tap the **upload/write icon** (usually an up arrow)
3. Select **"Text"** format
4. Enter your WiFi SSID (network name)
5. Tap **SEND**

#### Write Password:
1. Find characteristic with UUID: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
2. Tap the **upload/write icon**
3. Select **"Text"** format
4. Enter your WiFi password
5. Tap **SEND**

### Step 5: Monitor Status
1. Find the Status characteristic with UUID: `6E400004-B5A3-F393-E0A9-E50E24DCCA9E`
2. Tap the **notification icon** (usually multiple down arrows) to enable notifications
3. You'll see status updates like:
   - "Connected" (BLE connected)
   - "SSID received"
   - "Credentials received"
   - "Connecting to WiFi..."
   - "WiFi Connected!" (Success!)
   - Or "WiFi connection failed" (Retry needed)

## BLE Service & Characteristics

| Purpose | UUID | Properties |
|---------|------|------------|
| **Service** | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` | - |
| **SSID** | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | Write |
| **Password** | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | Write |
| **Status** | `6E400004-B5A3-F393-E0A9-E50E24DCCA9E` | Read, Notify |

## Troubleshooting

### Device Not Found in NRF Connect
- Make sure the ESP32 is powered on
- Check serial monitor for "BLE: Advertising started" message
- Move your phone closer to the ESP32
- Try refreshing the scan in NRF Connect

### Connection Failed
- Clear the device from NRF Connect's bonded devices
- Restart the ESP32
- Restart the NRF Connect app

### WiFi Connection Failed
- Double-check SSID spelling (case-sensitive!)
- Verify password is correct
- Make sure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Check if WiFi network is in range
- Try sending credentials again via BLE

### Can't See Characteristics
- Make sure you tapped "Discover services" after connecting
- Try disconnecting and reconnecting
- Restart the ESP32

## Serial Monitor Output

When running correctly, you should see:
```
========================================
  Smart Dustbin - BLE Provisioning
========================================

WiFi: Initialized
BLE: Initializing BLE...
BLE: Advertising started
BLE: Device name: SmartDustbin_ESP32
BLE: Ready for NRF Connect

>> Please use NRF Connect app to:
   1. Scan and connect to 'SmartDustbin_ESP32'
   2. Discover services
   3. Write SSID to characteristic 6E400002...
   4. Write Password to characteristic 6E400003...
   5. Device will auto-connect to WiFi

Waiting for WiFi credentials via BLE...
```

After successful provisioning:
```
BLE: Received SSID: YourNetworkName
BLE: Received Password: ********
========================================
Credentials received via BLE!
========================================
WiFi: Connecting...
WiFi: SSID: YourNetworkName
.......
WiFi: Connected successfully!
WiFi: IP Address: 192.168.1.XXX
========================================
  WiFi Connection Successful!
========================================
```

## Code Structure

```
├── include/
│   ├── ble_manager.h      # BLE function declarations
│   ├── wifi_manager.h     # WiFi function declarations
│   └── config.h           # Pin configurations
├── src/
│   ├── main.cpp           # Main program with provisioning logic
│   ├── ble_manager.cpp    # BLE implementation with callbacks
│   └── wifi_manager.cpp   # WiFi connection management
└── BLE_PROVISIONING.md    # This file
```

## Next Steps

After successful WiFi provisioning, you can:
1. Integrate with Firebase (credentials already in config.h)
2. Add ultrasonic sensor monitoring
3. Add servo control for dustbin lid
4. Implement your complete smart dustbin logic

## Notes

- BLE will continue advertising even after WiFi connection (you can disable this in code if needed)
- Credentials are stored in RAM only and will be lost on reboot
- For persistent storage, consider using ESP32 Preferences/EEPROM
- The device name "SmartDustbin_ESP32" can be changed in `ble_manager.cpp`

## Support

If you encounter issues:
1. Check the Serial Monitor at 115200 baud
2. Verify all libraries are installed (check platformio.ini)
3. Make sure you're using the correct UUIDs in NRF Connect
4. Try the troubleshooting steps above

---
**Happy Provisioning! 🎉**
