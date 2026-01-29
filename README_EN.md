# OPEC-SIMPLE-ESP32C14

🔥 **Heating system controller for ESP32-C3 with Modbus RTU communication**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino](https://img.shields.io/badge/Arduino-IDE-00979D?logo=arduino)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/ESP32-C3-E7352C?logo=espressif)](https://www.espressif.com/)

> Arduino project for **EletechSup ES32C14 (ESP32-C3)** board to control heating systems via Modbus RTU RS485. Conversion of the [OPEC-SIMPLE](https://github.com/IT-Buster/OPEC-SIMPLE) project (Node-RED) to native C++ code.

[🇵🇱 Wersja polska](README.md)

---

## 📖 Table of Contents

- [Project Description](#-project-description)
- [Main Features](#-main-features)
- [⚡ Modbus RS485 Pins](#-modbus-rs485-pins)
- [Requirements](#-requirements)
- [Installation](#-installation)
- [Initial Configuration](#-initial-configuration)
- [Usage](#-usage)
- [API Documentation](#-api-documentation)
- [Wiring Diagram](#-wiring-diagram)
- [Troubleshooting](#-troubleshooting)
- [Project Development](#-project-development)
- [License](#-license)

---

## 🎯 Project Description

**OPEC-SIMPLE-ESP32C14** is a complete heating control system based on the ESP32-C3 microcontroller that communicates with PLCs via Modbus RTU (RS485).

### What does the system do?

- 📊 **Reads data** from PLC via Modbus RTU (CO temperature, outdoor temperature, actuator position)
- 🌡️ **Controls temperature** using a thermostat with hysteresis algorithm
- 📈 **Uses heating curve** to automatically calculate target temperature based on outdoor temperature
- 🌐 **Offers web interface** for monitoring and configuration
- 📡 **Works in WiFi mode** AP (access point) or STA (network client)
- 💾 **Saves configuration** in Flash memory (Preferences)

### Why ESP32 instead of Node-RED?

- ✅ Autonomous controller without server dependencies
- ✅ Low power consumption
- ✅ Compact design (single module)
- ✅ Faster operation and greater reliability
- ✅ Lower total system cost

---

## ⭐ Main Features

### 1. Modbus RTU Communication (RS485)
- Uses built-in UART port in ES32C14
- Parameters: 115200 baud, 8N1, Unit ID: 2 (configurable)
- Read registers: actuator position, outdoor temperature, CO temperature
- Write: actuator control (0-100%)
- Automatic retry on errors (max 3 attempts)
- Reading every 15 seconds

### 2. Thermostat with Hysteresis
- Comparison of CO temperature with target temperature
- Hysteresis ±1°C (configurable)
- Algorithm:
  - TempCO < (Target - 1°C) → open actuator (100%)
  - TempCO > (Target + 1°C) → close actuator (0%)
  - Within hysteresis range → maintain state
- AUTO/MANUAL mode

### 3. Heating Curve
- Automatic calculation of target CO temperature based on outdoor temp.
- Linear interpolation between 9 curve points
- Default curve (editable via WebUI):

| Outdoor Temp. | CO Temp. |
|---------------|----------|
| -20°C         | 65°C     |
| -15°C         | 60°C     |
| -10°C         | 55°C     |
| -5°C          | 50°C     |
| 0°C           | 45°C     |
| 5°C           | 40°C     |
| 10°C          | 35°C     |
| 15°C          | 30°C     |
| 20°C          | 25°C     |

### 4. Web Interface (WebUI)
**Dashboard:**
- Real-time display:
  - CO temperature, outdoor temperature, target temperature
  - Actuator position (0-100%)
  - Thermostat status (ON/OFF)
  - WiFi and Modbus connection status
  - Uptime, free memory
- Auto-refresh every 5 seconds
- AUTO/MANUAL mode switch

**Heating Curve:**
- Table with editable curve points (9 points)
- "Save" button → save to Flash
- "Restore defaults" button

**Settings:**
- WiFi configuration (SSID, password)
- Modbus parameters (Unit ID, baudrate)
- Thermostat hysteresis (0.1 - 5.0°C)
- Factory reset

### 5. WiFi and Network
- **AP Mode:** On first start or when no saved network exists
  - SSID: `OPEC-ESP32-XXXX` (where XXXX = chip ID)
  - Password: `opec1234`
  - IP: `192.168.4.1`
- **STA Mode:** Connection to home WiFi network
- **mDNS:** Access via `http://opec-esp32.local`

---

## ⚡ Modbus RS485 Pins

### 🔌 Which ESP32 pins are used for Modbus communication?

The **EletechSup ES32C14** board uses the following ESP32-C3 GPIO pins:

| GPIO Pin | RS485 Function | Description |
|----------|----------------|-------------|
| **GPIO1** | **TX** (Transmit) | Sending data to Modbus device |
| **GPIO3** | **RX** (Receive) | Receiving data from Modbus device |
| **GPIO22** | **DE/RE** (Direction Enable) | Transmission direction control (TX/RX) |

### ⚠️ IMPORTANT NOTES:
- ES32C14 has a **built-in RS485 converter** - no need for external MAX485 module!
- **Serial Monitor will NOT work** during Modbus communication (shared pins with USB)
- Use Web UI for monitoring: `http://opec-esp32.local`
- GPIO1/GPIO3 pins are hardware-assigned to UART0 and cannot be changed

📖 **Detailed documentation:** [MODBUS_PINS_EN.md](MODBUS_PINS_EN.md) | [Polish version](MODBUS_PINS.md)

---

## 🔧 Hardware Specification

The project is designed for **EletechSup ES32C14** board (ESP32-C3) with built-in industrial interfaces:

### Key Board Features:
- ✅ **ESP32-C3** (RISC-V, WiFi, Bluetooth 5.0)
- ✅ **RS485 Modbus RTU** (IO1, IO3, IO22)
- ✅ **4x 10A Relays** (pump, valve control)
- ✅ **4x NPN Digital Inputs** (buttons, sensors)
- ✅ **Analog I/O** (0-10V, 0-20mA)
- ✅ Power supply **12V/24V DC** or **AC 85-265V**

### 📷 Board Documentation:

![ES32C14 Board](docs/images/es32c14-interfaces.avif)

**Full specification:** See [docs/ES32C14_HARDWARE_EN.md](docs/ES32C14_HARDWARE_EN.md)

**Wersja polska:** Zobacz [docs/ES32C14_HARDWARE.md](docs/ES32C14_HARDWARE.md)

---

### RS485 Pin Mapping (Modbus RTU):

| RS485 Function | ESP32 GPIO | ES32C14 Port |
|----------------|------------|--------------|
| TX (DI)        | GPIO 1     | IO1          |
| RX (RO)        | GPIO 3     | IO3          |
| DE/RE          | GPIO 22    | IO22         |

**Wiring Diagram:**
```
ES32C14 (IO1, IO3, IO22) → Built-in RS485 → A/B → PLC Modbus
                                               ↓
                                       120Ω Resistor (optional)
```

![Wiring Diagram](docs/images/es32c14-wiring.avif)

---

## 📋 Requirements

### Hardware
- **Microcontroller:** EletechSup ES32C14 (ESP32-C3) or compatible
- **RS485 Interface:** Built-in on ES32C14 board
- **Modbus PLC:** Device with Modbus RTU interface
- **Power Supply:** 12V/24V DC or AC 85-265V
- **Cables:** Shielded twisted pair for RS485

### Software
- **Arduino IDE:** 2.0 or newer
- **Board Support:** ESP32 by Espressif Systems
- **Libraries:** (see [libraries.txt](libraries.txt))
  - ModbusMaster (v2.0.1+)
  - ESPAsyncWebServer
  - AsyncTCP
  - ArduinoJson (v6.21.0+)
  - Preferences (built-in)
  - WiFi (built-in)
  - ESPmDNS (built-in)

---

## 🚀 Installation

### Step 1: Install Arduino IDE
1. Download [Arduino IDE](https://www.arduino.cc/en/software) (version 2.0+)
2. Install and launch

### Step 2: Add ESP32 Support
1. Open **File → Preferences**
2. In "Additional Board Manager URLs" field add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Click **OK**
4. Open **Tools → Board → Boards Manager**
5. Search for "ESP32" and install "**ESP32 by Espressif Systems**"

### Step 3: Install Libraries
1. Open **Tools → Manage Libraries** (Ctrl+Shift+I)
2. Install the following libraries:
   - **ModbusMaster** by Doc Walker
   - **ArduinoJson** by Benoit Blanchon (v6.x)

3. Install libraries from GitHub (manually):
   
   **ESPAsyncWebServer:**
   ```bash
   cd ~/Arduino/libraries
   git clone https://github.com/me-no-dev/ESPAsyncWebServer.git
   ```
   
   **AsyncTCP:**
   ```bash
   cd ~/Arduino/libraries
   git clone https://github.com/me-no-dev/AsyncTCP.git
   ```

### Step 4: Download Project
```bash
git clone https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14.git
cd OPEC-SIMPLE-ESP32C14
```

Alternatively: Download ZIP from GitHub → **Code → Download ZIP**

### Step 5: Configure Arduino IDE
1. Open `OPEC-SIMPLE-ESP32C14.ino` file in Arduino IDE
2. Select board:
   - **Tools → Board → ESP32 Arduino → ESP32C3 Dev Module**
3. Set parameters:
   - **Upload Speed:** 921600
   - **CPU Frequency:** 160MHz
   - **Flash Size:** 4MB (32Mb)
   - **Partition Scheme:** Default 4MB (without SPIFFS)
   - **Core Debug Level:** None (for release) or Info (for debugging)

### Step 6: Upload Code
1. Connect ESP32 via USB
2. Select appropriate port: **Tools → Port → COMX** (Windows) or **/dev/ttyUSBX** (Linux)
3. Click **Upload** (Ctrl+U)
4. Wait for completion (approx. 30-60 seconds)
5. **Done!** WebUI is embedded in code - no need to upload SPIFFS files

### Step 7: Connect Hardware
Connect ESP32 to RS485 module according to the diagram in [wiring_diagram.md](wiring_diagram.md)

---

## ⚙️ Initial Configuration

### First Boot

1. **Power:** Connect ESP32 to power supply (USB or 5V external)

2. **AP Mode:** ESP32 will automatically create a WiFi access point:
   - SSID: `OPEC-ESP32-XXXX` (where XXXX is chip ID)
   - Password: `opec1234`
   - IP: `192.168.4.1`

3. **Connection:** Connect to ESP32 WiFi using phone or computer

4. **WebUI:** Open browser and navigate to `http://192.168.4.1`

### Configure Home WiFi

1. Go to **Settings** in WebUI menu
2. Enter **SSID** and **password** of your WiFi network
3. Click **Save settings**
4. ESP32 will restart and connect to your network

### Configure Modbus

1. In **Settings** section set:
   - **Unit ID:** Modbus PLC address (default: 2)
   - **Baudrate:** Transmission speed (default: 115200)
2. Click **Save settings**
3. Restart ESP32 (disconnect and reconnect power)

### Set Heating Curve

1. Go to **Curve** in WebUI menu
2. Adjust curve points as needed
3. Click **Save curve**

---

## 💡 Usage

### Access WebUI

After configuring WiFi, access the controller via:

- **mDNS:** `http://opec-esp32.local`
- **IP:** Check IP in router or Serial Monitor

### Dashboard

Dashboard displays in real-time:
- 🌡️ CO temperature
- 🌡️ Outdoor temperature
- 🎯 Target temperature (calculated from curve)
- 🔧 Actuator position (0-100%)
- ✅ Thermostat status (ON/OFF)
- 📡 Modbus status
- 📶 WiFi quality
- ⏱️ Uptime
- 💾 Free memory

**Auto-refresh:** Every 5 seconds

### Thermostat Control

**AUTO Mode:**
- Thermostat active
- Target temperature calculated from heating curve
- Automatic actuator control

**MANUAL Mode:**
- Thermostat disabled
- Actuator controlled manually (via WebUI or externally)

**Switching:** Use toggle switch on Dashboard

### Edit Heating Curve

1. Go to **Curve**
2. Edit values in table
3. Click **Save curve**
4. New curve will be applied immediately

**Restore default curve:**
- Click **Restore defaults**
- Confirm
- Click **Save curve**

### Change Settings

1. Go to **Settings**
2. Change selected parameters:
   - WiFi SSID/password
   - Modbus Unit ID/Baudrate
   - Thermostat hysteresis
3. Click **Save settings**
4. Restart ESP32 if you changed WiFi or Modbus

### Factory Reset

1. **Via WebUI:**
   - Go to **Settings**
   - Click **Factory reset**
   - Confirm
   - ESP32 will restart

2. **Via BOOT button:**
   - Hold BOOT button for 5 seconds during startup
   - ESP32 will reset to default settings

---

## 🔌 API Documentation

System offers REST API for integration with external applications.

### GET /api/status

Returns current system state.

**Example response:**
```json
{
  "tempCO": 45.5,
  "tempEXT": 8.2,
  "tempTarget": 42.0,
  "actuatorPos": 65,
  "thermostatActive": true,
  "modbusConnected": true,
  "wifiRSSI": -45,
  "uptime": 3600,
  "freeHeap": 245760,
  "chipModel": "ESP32-C3"
}
```

**Example curl:**
```bash
curl http://opec-esp32.local/api/status
```

### GET /api/curve

Returns current heating curve.

**Example response:**
```json
{
  "curve": [
    [-20, 65],
    [-15, 60],
    [-10, 55],
    [-5, 50],
    [0, 45],
    [5, 40],
    [10, 35],
    [15, 30],
    [20, 25]
  ]
}
```

### POST /api/curve

Updates heating curve.

**Example request:**
```bash
curl -X POST http://opec-esp32.local/api/curve \
  -H "Content-Type: application/json" \
  -d '{"curve":[[-20,65],[-15,60],[-10,55],[-5,50],[0,45],[5,40],[10,35],[15,30],[20,25]]}'
```

### POST /api/thermostat

Enables/disables thermostat.

**Example request:**
```bash
# Enable thermostat
curl -X POST http://opec-esp32.local/api/thermostat \
  -H "Content-Type: application/json" \
  -d '{"active":true}'

# Disable thermostat
curl -X POST http://opec-esp32.local/api/thermostat \
  -H "Content-Type: application/json" \
  -d '{"active":false}'
```

### GET /api/settings

Returns current settings (without WiFi password).

**Example response:**
```json
{
  "wifiSSID": "MyWiFi",
  "modbusUnitID": 2,
  "modbusBaudrate": 115200,
  "hysteresis": 1.0,
  "thermostatActive": true
}
```

### POST /api/settings

Updates system settings.

**Example request:**
```bash
curl -X POST http://opec-esp32.local/api/settings \
  -H "Content-Type: application/json" \
  -d '{"wifiSSID":"NewWiFi","wifiPassword":"password123","hysteresis":1.5}'
```

### POST /api/reset

Resets settings to factory defaults and restarts device.

**Example request:**
```bash
curl -X POST http://opec-esp32.local/api/reset
```

---

## ⚡ Wiring Diagram

Detailed wiring diagram is available in [wiring_diagram.md](wiring_diagram.md).

### ESP32-C3 (ES32C14) Pins

| GPIO Pin | Function | Description             |
|----------|----------|-------------------------|
| GPIO 1   | TX       | UART TX (RS485 DI)      |
| GPIO 3   | RX       | UART RX (RS485 RO)      |
| GPIO 22  | DE/RE    | Direction Enable RS485  |
| 12-24V   | VCC      | Power Supply            |
| GND      | GND      | Ground                  |

### Connection to Modbus PLC

```
ES32C14 Built-in RS485 → PLC
A (+)                   → A
B (-)                   → B
GND                     → GND
```

**⚠️ Important:**
- ES32C14 has **built-in RS485 converter** - no external MAX485 module needed
- Add 120Ω terminating resistor between A and B if ESP32 is at the end of the line
- Use shielded twisted pair for noise immunity
- Make sure all devices share common ground (GND)

---

## 🔧 Troubleshooting

### No WiFi Connection

**Problem:** ESP32 doesn't connect to WiFi network

**Solution:**
1. Check SSID and password in settings
2. Make sure WiFi network is in range
3. Check if router supports 2.4GHz WiFi (ESP32 doesn't support 5GHz)
4. Try factory reset
5. Check Serial Monitor (115200 baud) for detailed logs

### Modbus Errors

**Problem:** No communication with PLC, read errors

**Solution:**
1. Check RS485 wiring (A, B, GND)
2. Try swapping A ↔ B wires
3. Check Unit ID in settings (must match PLC)
4. Check baudrate (must match PLC)
5. Add 120Ω terminating resistor between A and B
6. Check RS485 module power supply
7. Use Modbus Scanner to test connection
8. Check logs in Serial Monitor

### WebUI Not Loading

**Problem:** Page doesn't display or returns 404 error

**Solution:**
1. Make sure you uploaded the latest code (WebUI is embedded in code since v1.1+)
2. Check Serial Monitor - should show "Strony HTML wbudowane w kod (bez SPIFFS)"
3. Try using IP address instead of mDNS
4. Clear browser cache (Ctrl+F5)
5. If using older code version, update to latest

### Thermostat Not Working

**Problem:** Actuator doesn't respond to temperature changes

**Solution:**
1. Check if thermostat is enabled (AUTO mode)
2. Check Modbus connection (are data being read)
3. Check heating curve (are temperatures correct)
4. Check hysteresis (not too large)
5. Check logs in Serial Monitor for thermostat algorithm
6. Check outdoor temperature range (must be within curve)

### System Freezes

**Problem:** ESP32 resets or freezes

**Solution:**
1. Check power supply (min. 500mA, stabilized 5V)
2. Check logs for Stack Overflow or Watchdog Reset
3. Increase delays in code (if modified)
4. Check ESP32 temperature (shouldn't exceed 80°C)
5. Re-upload code with Debug Level set to "Verbose"

### mDNS Not Working

**Problem:** Cannot access via `opec-esp32.local`

**Solution:**
1. Make sure you're on the same network as ESP32
2. Check if your system supports mDNS:
   - **Windows:** Install Bonjour Print Services
   - **Linux:** Install Avahi
   - **macOS:** Works natively
3. Use direct IP address instead of mDNS
4. Check Serial Monitor for mDNS status

---

## 🚀 Project Development

### Planned Features

- [ ] **MQTT:** Integration with Home Assistant, Domoticz
- [ ] **ThingSpeak:** Cloud data upload
- [ ] **OTA Update:** Firmware update over WiFi
- [ ] **Schedule:** Temperature planning for different times of day
- [ ] **Multi-zone:** Support for multiple heating zones
- [ ] **Mobile App:** Dedicated mobile application
- [ ] **Notifications:** Email/SMS on errors
- [ ] **Charts:** Temperature history visualization

### How to Contribute

We welcome Pull Requests! 🎉

1. Fork the repository
2. Create a branch for your feature (`git checkout -b feature/NewFeature`)
3. Commit changes (`git commit -m 'Add NewFeature'`)
4. Push to branch (`git push origin feature/NewFeature`)
5. Open Pull Request

### Report Bugs

Found a bug? Report it via [GitHub Issues](https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14/issues)

Remember to include:
- Arduino IDE and ESP32 Core version
- Logs from Serial Monitor
- Steps to reproduce the problem

---

## 📄 License

This project is licensed under the **MIT License**.

```
MIT License

Copyright (c) 2026 IT-Buster

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 👤 Author

**IT-Buster**
- GitHub: [@IT-Buster](https://github.com/IT-Buster)
- Based on: [OPEC-SIMPLE](https://github.com/IT-Buster/OPEC-SIMPLE) (Node-RED)

---

## 🙏 Acknowledgements

- [ModbusMaster](https://github.com/4-20ma/ModbusMaster) by Doc Walker
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) by me-no-dev
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) by Benoit Blanchon
- ESP32 and Arduino community

---

**⭐ If you like this project, give it a star on GitHub! ⭐**