# BLE WiFi Provisioning - Quick Start Guide

## 🎯 What We Implemented

Following the NimBLE example pattern, we've implemented a complete BLE WiFi provisioning system for your Smart Dustbin ESP32 project.

## 📁 Files Created/Modified

### New Files:
- **`src/ble_manager.cpp`** - Complete BLE server implementation with:
  - Server callbacks (connection/disconnection handling)
  - Characteristic callbacks (SSID/Password write handling)
  - Status notifications
  - Credential management

### Modified Files:
- **`src/main.cpp`** - Main application logic with:
  - BLE initialization
  - WiFi provisioning flow
  - Connection monitoring
  - Status updates

### Existing Files Used:
- **`include/ble_manager.h`** - BLE function declarations and UUIDs
- **`include/wifi_manager.h`** - WiFi connection functions
- **`src/wifi_manager.cpp`** - WiFi implementation

## 🚀 How to Use

### 1. Upload the Code
```bash
pio run --target upload
pio device monitor
```

### 2. Open Serial Monitor
You should see:
```
========================================
   Smart Dustbin ESP32 - Starting
========================================

Step 1: Initializing WiFi...
WiFi: Initialized

Step 2: Initializing BLE provisioning...
BLE: Initializing...
BLE: Initialized successfully
BLE: Advertising started - Device name: SmartDustbin_ESP32
BLE: Waiting for WiFi credentials...

========================================
System ready!
========================================
Use NRF Connect app to provision WiFi
Device name: SmartDustbin_ESP32
========================================
```

### 3. Use NRF Connect App

#### Download:
- **Android**: [Google Play](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)
- **iOS**: [App Store](https://apps.apple.com/app/nrf-connect/id1054362403)

#### Steps:
1. **Scan** for devices
2. **Connect** to "SmartDustbin_ESP32"
3. **Discover services** (wait for service discovery)
4. Find the service: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`

#### Write Credentials:
5. **SSID Characteristic** (`6E400002...`):
   - Tap write icon
   - Select "Text"
   - Enter WiFi name
   - Send

6. **Password Characteristic** (`6E400003...`):
   - Tap write icon
   - Select "Text"
   - Enter WiFi password
   - Send

7. **Monitor Status** (`6E400004...`):
   - Tap notification icon
   - Watch for status updates:
     - "Connected" (BLE connected)
     - "SSID received"
     - "Credentials received"
     - "Connecting to WiFi..."
     - "WiFi Connected!" ✓

## 🔧 Key Features Implemented

### Following NimBLE Example Pattern:
✅ **Server Callbacks** - Connection management like the example
✅ **Characteristic Callbacks** - Read/write handling
✅ **Notifications** - Status updates to connected clients
✅ **Connection Parameters** - Optimized BLE performance
✅ **Advertising** - Proper service UUID advertising

### WiFi Provisioning Features:
✅ **Credential Reception** - SSID and Password via BLE
✅ **WiFi Connection** - Automatic connection attempt
✅ **Status Feedback** - Real-time updates via BLE notifications
✅ **Retry Logic** - Clear credentials on failure for retry
✅ **Connection Monitoring** - Periodic WiFi health checks

## 📊 BLE Service Structure

```
Service: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E (Nordic UART Service)
├── SSID Characteristic: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
│   └── Properties: WRITE
├── Password Characteristic: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E
│   └── Properties: WRITE
└── Status Characteristic: 6E400004-B5A3-F393-E0A9-E50E24DCCA9E
    └── Properties: READ, NOTIFY
```

## 🔍 What Happens Behind the Scenes

1. **BLE Server starts** advertising as "SmartDustbin_ESP32"
2. **Phone connects** → `onConnect()` callback fires
3. **SSID written** → Stored in memory, status updated
4. **Password written** → Stored in memory, `hasCredentials()` returns true
5. **Main loop detects** credentials → Attempts WiFi connection
6. **Success/Failure** → Status sent via BLE notification
7. **Monitor loop** → Checks WiFi health every 10 seconds

## 🎨 Customization Options

### Change Device Name:
In `ble_manager.cpp`, line ~152:
```cpp
NimBLEDevice::init("SmartDustbin_ESP32");  // Change here
```

### Add Security (Optional):
In `ble_manager.cpp`, uncomment line ~155:
```cpp
NimBLEDevice::setSecurityAuth(false, false, true);
```

### Auto-clear Credentials:
In `main.cpp`, line ~66, uncomment:
```cpp
clearCredentials();  // Clears from memory after connection
```

## 🐛 Troubleshooting

### Device not found?
- Check serial monitor for "BLE: Advertising started"
- Restart ESP32
- Move phone closer

### WiFi won't connect?
- Check SSID spelling (case-sensitive!)
- Verify password
- Check serial monitor for detailed errors

### BLE disconnects immediately?
- Normal after WiFi connects (provisioning complete)
- Can keep BLE running if needed

## 📚 Next Steps

After WiFi is connected, you can:
- Add Firebase connection code
- Integrate servo and ultrasonic sensors
- Implement your dustbin logic
- Keep BLE active for status monitoring (optional)

## 🎓 Learning Points from NimBLE Example

The implementation follows these patterns from the official example:

1. **Callback Classes** - Separate classes for server and characteristic callbacks
2. **Connection Management** - Proper handling of connect/disconnect events
3. **Service Creation** - Creating services with multiple characteristics
4. **Advertising** - Setting up advertising with service UUIDs
5. **Notifications** - Implementing notify functionality for status updates

Enjoy your BLE-provisioned Smart Dustbin! 🗑️✨
