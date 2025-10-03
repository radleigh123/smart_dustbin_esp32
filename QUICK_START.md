# Quick Start Guide - BLE Provisioning

## 🚀 Quick Steps

### 1. Upload Code to ESP32
- Connect ESP32 via USB
- Upload the code using PlatformIO
- Open Serial Monitor (115200 baud)

### 2. Connect via NRF Connect App
- Open NRF Connect on your phone
- Scan for "SmartDustbin_ESP32"
- Tap CONNECT

### 3. Send WiFi Credentials
Find the service `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`

**Write SSID:**
- Characteristic: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- Format: Text
- Value: Your WiFi network name

**Write Password:**
- Characteristic: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- Format: Text
- Value: Your WiFi password

### 4. Monitor Connection
- Enable notifications on characteristic `6E400004-B5A3-F393-E0A9-E50E24DCCA9E`
- Watch for "WiFi Connected!" status

## 📱 NRF Connect Screenshots Flow

1. **Scan** → Look for "SmartDustbin_ESP32"
2. **Connect** → Tap the CONNECT button
3. **Discover** → Services will auto-discover
4. **Expand** → Open service `6E40...CA9E`
5. **Write SSID** → Tap upload icon on `6E40...CA9E` (2nd char)
6. **Write Pass** → Tap upload icon on `6E40...CA9E` (3rd char)
7. **Monitor** → Enable notifications on `6E40...CA9E` (4th char)

## ⚡ Expected Serial Output

```
========================================
  Smart Dustbin - BLE Provisioning
========================================

BLE: Advertising started
BLE: Device connected
BLE: Received SSID: YourNetwork
BLE: Received Password: ********
WiFi: Connecting...
WiFi: Connected successfully!
WiFi: IP Address: 192.168.1.XXX
```

## 🔧 Common Issues

| Problem | Solution |
|---------|----------|
| Device not found | Check ESP32 power, restart NRF app |
| Can't connect | Clear bonded devices in NRF |
| WiFi fails | Check SSID/password, must be 2.4GHz |
| No characteristics | Tap "Discover services" |

## 📝 UUIDs Reference

```
Service:  6E400001-B5A3-F393-E0A9-E50E24DCCA9E
SSID:     6E400002-B5A3-F393-E0A9-E50E24DCCA9E (Write)
Password: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E (Write)
Status:   6E400004-B5A3-F393-E0A9-E50E24DCCA9E (Read/Notify)
```

---
✅ **Ready to provision your Smart Dustbin!**
