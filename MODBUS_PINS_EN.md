# 📌 Modbus RS485 Pins for ESP32C14

## Question: Which pins are used in ESP32 for Modbus communication?

### ✅ Answer:

The **EletechSup ES32C14** board uses the following ESP32-C3 GPIO pins for Modbus RTU communication via RS485:

| GPIO Pin | RS485 Function | Description |
|----------|----------------|-------------|
| **GPIO1** | **TX** (Transmit) | Sending data to Modbus device |
| **GPIO3** | **RX** (Receive) | Receiving data from Modbus device |
| **GPIO22** | **DE/RE** (Direction Enable) | Transmission direction control (TX/RX) |

---

## 🔧 Technical Details

### 1. UART0 (Serial)
- ES32C14 uses **UART0** (the `Serial` object in Arduino) for RS485 communication
- GPIO1 (TX) and GPIO3 (RX) are the **default UART0 pins** on ESP32-C3
- These pins are **hardware-assigned** and cannot be changed

### 2. DE/RE Pin (GPIO22)
- **DE** (Driver Enable) - enables RS485 transmitter
- **RE** (Receiver Enable) - enables RS485 receiver
- GPIO22 controls the operating mode:
  - `HIGH` → Transmit mode (TX)
  - `LOW` → Receive mode (RX)

### 3. Built-in RS485 Transceiver
ES32C14 has a **built-in RS485 converter** connected to these pins - **you don't need an external MAX485 module!**

---

## 💻 Source Code

### Definition in `config.h`:
```cpp
// RS485 uses Serial (UART0) - default TX(GPIO1)/RX(GPIO3) pins
// Only DE/RE requires definition
#define RS485_DE_RE_PIN  22  // IO22 → DE+RE (Direction Enable)
```

### Initialization in `modbus_handler.cpp`:
```cpp
void setupModbus() {
  // UART0 (Serial) with GPIO1/GPIO3 pins
  Serial.begin(config.modbusBaudrate, SERIAL_8N1);
  Serial.setTimeout(5);
  
  // Configure DE/RE pin (GPIO 22)
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW); // RX mode (receive)
  
  // Initialize ModbusMaster library
  modbus.begin(config.modbusUnitID, Serial);
  
  // Callbacks for TX/RX switching
  modbus.preTransmission(preTransmission);
  modbus.postTransmission(postTransmission);
}
```

---

## ⚠️ IMPORTANT NOTES

### 1. Serial Monitor Will NOT Work
- **GPIO1/GPIO3** are used by both USB (Serial Monitor) and RS485
- During Modbus communication, **Serial Monitor will not be available**
- Use the **Web UI** for system monitoring: `http://opec-esp32.local`

### 2. Pins Cannot Be Changed
- GPIO1 (TX) and GPIO3 (RX) are **hardware-assigned to UART0**
- These are the only pins supporting serial communication in ESP32-C3
- ES32C14 uses this hardware configuration

### 3. Shared with USB
- The same pins handle both USB communication (programming) and RS485
- During normal operation, choose one: **programming** OR **Modbus**
- Cannot use both simultaneously

---

## 🔌 Physical Connections

### P13 Connector (RS485) on ES32C14 Board:

```
┌──────────────────────────────────┐
│  ES32C14 Board                   │
│                                   │
│  ESP32-C3                         │
│  GPIO1 (TX) ──┐                  │
│  GPIO3 (RX) ──┼─► RS485          │
│  GPIO22     ──┘   Transceiver    │
│                        │          │
│                        ▼          │
│  ┌─────────────────────────┐     │
│  │ P13 (RS485 Connector)   │     │
│  │  [A]  [B]  [GND]        │     │
│  └──┬────┬─────┬───────────┘     │
└─────┼────┼─────┼──────────────────┘
      │    │     │
      A    B    GND
      │    │     │
      ▼    ▼     ▼
  To Modbus device (PLC, HT73, etc.)
```

### Connection to Modbus Device:

| ES32C14 P13 | → | Modbus Device        |
|-------------|---|----------------------|
| A           | → | A (or +, Data+)      |
| B           | → | B (or -, Data-)      |
| GND         | → | GND (ground)         |

---

## 📚 Additional Resources

- **Wiring Diagram:** [wiring_diagram.md](wiring_diagram.md)
- **Modbus Configuration:** [ADVANCED_MODBUS_CONFIG.md](ADVANCED_MODBUS_CONFIG.md)
- **Header File:** [config.h](config.h) (lines 23-41)
- **Implementation:** [modbus_handler.cpp](modbus_handler.cpp)
- **Polish Version:** [MODBUS_PINS.md](MODBUS_PINS.md)

---

## 🎯 Summary

**Pins used for Modbus RS485 communication:**
- ✅ **GPIO1** - TX (data transmission)
- ✅ **GPIO3** - RX (data reception)
- ✅ **GPIO22** - DE/RE (direction control)

**Hardware Platform:**
- Board: **EletechSup ES32C14**
- Chip: **ESP32-C3**
- Interface: **Built-in RS485 transceiver**
- UART: **UART0 (Serial)**

---

*Document created: 2026-01-29*  
*Author: IT-Buster*  
*Firmware version: 1.2.0*
