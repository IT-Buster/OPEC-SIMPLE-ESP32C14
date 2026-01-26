# 🔧 Hardware Specification - EleTechSup ES32C14

## Board Overview

EleTechSup ES32C14 is an advanced development board based on **ESP32-C3** with built-in industrial interfaces:

- ✅ **RS485** (Modbus RTU)
- ✅ **4x 10A Relays** (AC/DC load control)
- ✅ **4x Digital Inputs** (NPN, 24V)
- ✅ **2x Analog Inputs 0-10V**
- ✅ **2x Analog Inputs 0-20mA**
- ✅ **2x Analog Outputs 0-10V / 0-20mA** (multiplexed)
- ✅ Power supply: **12V/24V DC** or **AC 85-265V**

---

## 📌 Pin Mapping

### RS485 Port (Modbus RTU)
| ES32C14 Port | ESP32 GPIO | RS485 Function | Description |
|--------------|------------|----------------|-------------|
| IO1          | GPIO 1     | DI (TX)        | Data Input (transmit to RS485) |
| IO3          | GPIO 3     | RO (RX)        | Receive Output (receive from RS485) |
| IO22         | GPIO 22    | DE + RE        | Direction Enable (control direction) |

**Connection to PLC Modbus:**
- IO1 (GPIO1) → RS485 Module → A/B to PLC
- IO3 (GPIO3) → RS485 Module → A/B from PLC
- IO22 (GPIO22) → Control DE/RE of RS485 module

---

### Relay Outputs (10A, 250V AC)
| Channel | ESP32 GPIO | Connector | Maximum Load |
|---------|------------|-----------|--------------|
| CH1     | IO27       | RELAY1    | 10A @ 250V AC / 30V DC |
| CH2     | IO14       | RELAY2    | 10A @ 250V AC / 30V DC |
| CH3     | IO12       | RELAY3    | 10A @ 250V AC / 30V DC |
| CH4     | IO13       | RELAY4    | 10A @ 250V AC / 30V DC |

**Example Applications:**
- Heating circulation pump control
- 3-way valves
- Signaling (LED lights, buzzer)

---

### Digital Inputs NPN (24V)
| Input | ESP32 GPIO | Type | Voltage |
|-------|------------|------|---------|
| IN1   | IO19       | NPN  | 0-24V DC |
| IN2   | IO18       | NPN  | 0-24V DC |
| IN3   | IO5        | NPN  | 0-24V DC |
| IN4   | IO17       | NPN  | 0-24V DC |

**Example Applications:**
- RESET button
- Window open sensor
- PIR sensor (presence detection)
- Mode switch

---

### Analog Inputs
| Input | ESP32 GPIO | Range  | Resolution |
|-------|------------|--------|------------|
| Vi1   | IO36       | 0-10V  | 12-bit ADC |
| Vi2   | IO39       | 0-10V  | 12-bit ADC |
| Ii1   | IO34       | 0-20mA | 12-bit ADC |
| Ii2   | IO35       | 0-20mA | 12-bit ADC |

**Example Applications:**
- 0-10V temperature sensor (alternative to Modbus)
- 4-20mA pressure sensor
- Light intensity (LUX)

---

### Analog Outputs (MULTIPLEXED!)
| Output     | ESP32 GPIO | Range          | Mode Selection  |
|------------|------------|----------------|-----------------|
| Vo1 / Io1  | IO25/IO26  | 0-10V / 0-20mA | DIP switch SW1 |
| Vo2 / Io2  | IO25/IO26  | 0-10V / 0-20mA | DIP switch SW1 |

⚠️ **WARNING:** Only **one mode** (Vo or Io) can be active at a time!
- Set DIP switch SW1 according to your needs:
  - Vo1/Vo2: Voltage output 0-10V
  - Io1/Io2: Current output 0-20mA

**Example Applications:**
- 0-10V actuator control (HVAC)
- 4-20mA inverter control

---

### Free GPIO Ports
Available for use in your project:
```
IO33, IO32, IO15, IO2, IO0, IO4, IO16, IO21, IO23
```

**Example Applications:**
- DHT22 sensors (temperature/humidity)
- OLED I2C display
- Additional status LEDs
- PIR motion sensor

---

## 🔌 Power Supply

### Option 1: DC 12V/24V (Connector P1/P2)
| Pin | Function |
|-----|----------|
| +V  | 12V or 24V DC |
| GND | Ground |

### Option 2: AC 85-265V (built-in power supply)
The board has a built-in AC/DC power supply allowing direct connection to 230V AC mains.

⚠️ **Warning:** High voltage! AC installation should be performed by a qualified electrician!

---

## 📷 Photos and Diagrams

### Board Pinout
![ES32C14 Pinout](images/es32c14-pinout.avif)

### Interface Description
![ES32C14 Interfaces](images/es32c14-interfaces.avif)

### Wiring Diagram
![ES32C14 Wiring](images/es32c14-wiring.avif)

### DIP Switch Configuration
![ES32C14 DIP Switch](images/es32c14-dip-switch.avif)

---

## 🛠️ Application in OPEC Project

### Minimal Configuration (Modbus + Thermostat only):
- **RS485:** IO1, IO3, IO22 → Communication with PLC
- **Relay CH1:** IO27 → Heating pump control (optional)
- **LED Status:** IO2 → Thermostat operation signaling

### Extended Configuration (with additional sensors):
- **RS485:** Modbus RTU to PLC
- **Relay CH1-CH4:** Pump, valve, fan, alarm
- **IN1-IN2:** Mode buttons (AUTO/MANUAL)
- **Vi1:** External temperature sensor 0-10V (Modbus backup)
- **Ii1:** Heating system pressure sensor (4-20mA)

---

## 🔗 Links

- **Manufacturer Documentation:** [EleTechSup ES32C14](https://www.eletechsup.com/)
- **ESP32-C3 Datasheet:** [Espressif ESP32-C3 Product Page](https://www.espressif.com/en/products/socs/esp32-c3)
- **Arduino Code:** [OPEC-SIMPLE-ESP32C14](https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14)

---

## 📝 License

Documentation available under MIT License - see LICENSE file in the main project directory.
