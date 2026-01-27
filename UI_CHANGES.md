# UI Changes - Visual Guide

## Settings Page - New Advanced Configuration Section

### BEFORE:
```
┌─────────────────────────────────────────┐
│ 🔌 Konfiguracja Modbus                  │
├─────────────────────────────────────────┤
│ Baudrate: [dropdown]                    │
│ Unit ID: [input]                        │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ 🌡️ Ustawienia termostatu               │
├─────────────────────────────────────────┤
│ Histereza: [input]                      │
│ ☑ Termostat aktywny                     │
└─────────────────────────────────────────┘
```

### AFTER:
```
┌─────────────────────────────────────────┐
│ 🔌 Konfiguracja Modbus                  │
├─────────────────────────────────────────┤
│ Baudrate: [dropdown]                    │
│ Unit ID: [input]                        │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐ ← NEW SECTION
│ 🔧 Konfiguracja Modbus - Zaawansowane   │
├─────────────────────────────────────────┤
│ Typ rejestru:                           │
│ [Holding Registers (FC 0x03)      ▼]    │
│                                         │
│ Adres startowy rejestru:                │
│ [0                                 ]    │
│                                         │
│ Liczba rejestrów do odczytu:            │
│ [2                                 ]    │
│                                         │
│ Mapowanie danych:                       │
│ [HT73 (Reg0=Wilgotność, Reg1=Temp) ▼]  │
│                                         │
│ Dzielnik wartości:                      │
│ [÷10 (230 → 23.0)                  ▼]  │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│ 🌡️ Ustawienia termostatu               │
├─────────────────────────────────────────┤
│ Histereza: [input]                      │
│ ☑ Termostat aktywny                     │
└─────────────────────────────────────────┘
```

## Diagnostics Page - New Testing Tools

### BEFORE:
```
┌──────────────────────────────────────────────────┐
│ 📊 Status połączenia                             │
├──────────────────────────────────────────────────┤
│ Slave ID: 5                                      │
│ Status: ✅ Połączony                             │
│ Ostatni odczyt: 14:23:15                         │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 📋 Surowe wartości rejestrów                     │
├──────────────────────────────────────────────────┤
│ Rej │ HEX    │ Dec │ Binary │ Int16 │ /10  │Opis│
├─────┼────────┼─────┼────────┼───────┼──────┼────┤
│  0  │ 0x0230 │ 560 │ ...    │  560  │ 56.0 │Wilg│
│  1  │ 0x00E6 │ 230 │ ...    │  230  │ 23.0 │Temp│
└──────────────────────────────────────────────────┘
```

### AFTER:
```
┌──────────────────────────────────────────────────┐
│ 📊 Status połączenia                             │
├──────────────────────────────────────────────────┤
│ Slave ID: 5                                      │
│ Status: ✅ Połączony                             │
│ Ostatni odczyt: 14:23:15                         │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐ ← NEW SECTION
│ 🔬 Narzędzia testowe Modbus                      │
├──────────────────────────────────────────────────┤
│ [🔍 Skanuj Slave ID]  [📖 Test FC03]  [📖 Test FC04]│
│                                                  │
│ ┌────────────────────────────────────────────┐  │
│ │ Wyniki skanowania:                         │  │
│ │                                            │  │
│ │ Znalezione urządzenia:                     │  │
│ │                                            │  │
│ │ Slave ID 5:                                │  │
│ │   - FC03 (Holding): ✅ OK                  │  │
│ │   - FC04 (Input):   ❌ Brak                │  │
│ └────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐ ← NEW SECTION
│ 📊 Informacje diagnostyczne                     │
├──────────────────────────────────────────────────┤
│ Typ odczytu: Holding Registers (FC03)            │
│ Adres startowy: 0                                │
│ Liczba rejestrów: 2                              │
│ Mapowanie: HT73 (Reg0=Wilg, Reg1=Temp)           │
│ Dzielnik: ÷10                                    │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 📋 Surowe wartości rejestrów                     │
├──────────────────────────────────────────────────┤
│Rej│ HEX    │ Dec │Int16│ /10  │ /100 │   Opis   │← UPDATED
├───┼────────┼─────┼─────┼──────┼──────┼──────────┤
│ 0 │ 0x0230 │ 560 │ 560 │ 56.0 │ 5.60 │Wilg × 10 │
│ 1 │ 0x00E6 │ 230 │ 230 │ 23.0 │ 2.30 │Temp × 10 │
│ 2 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │
│ 3 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │
│ 4 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │
│ 5 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │
│ 6 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │
│ 7 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │
│ 8 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │
│ 9 │ 0x0000 │   0 │   0 │  0.0 │ 0.00 │    -     │← NOW 10 ROWS
└──────────────────────────────────────────────────┘
```

## API Endpoints

### NEW Endpoints:

```
GET /api/modbus-scan
Response:
{
  "scanned": [
    {
      "id": 5,
      "fc03": true,
      "fc04": false
    }
  ]
}
```

```
GET /api/modbus-test?fc=3
Response:
{
  "type": "Holding Registers (FC03)",
  "success": true,
  "errorCode": 0,
  "registers": [560, 230]
}
```

```
GET /api/modbus-test?fc=4
Response:
{
  "type": "Input Registers (FC04)",
  "success": false,
  "errorCode": 2
}
```

### UPDATED Endpoints:

```
GET /api/settings
Response (extended):
{
  "wifiSSID": "MyWiFi",
  "modbusUnitID": 5,
  "modbusBaudrate": 9600,
  "modbusUseInputRegisters": false,    ← existed before
  "hysteresis": 1.0,
  "thermostatActive": true,
  
  "modbusStartRegister": 0,            ← NEW
  "modbusRegisterCount": 2,            ← NEW
  "modbusDataMapping": 0,              ← NEW
  "modbusDivider": 1                   ← NEW
}
```

```
POST /api/settings
Request (extended):
{
  "wifiSSID": "MyWiFi",
  "modbusUnitID": 5,
  "modbusBaudrate": 9600,
  "modbusUseInputRegisters": false,
  "modbusStartRegister": 0,            ← NEW
  "modbusRegisterCount": 2,            ← NEW
  "modbusDataMapping": 0,              ← NEW
  "modbusDivider": 1                   ← NEW
}
```

```
GET /api/modbus-raw
Response (extended):
{
  "slaveID": 5,
  "connected": true,
  "lastUpdate": 123456,
  "registers": [                       ← NOW 10 ITEMS (was 2-6)
    {
      "index": 0,
      "hex": "0x0230",
      "decimal": 560,
      "int16": 560,
      "float": 56.0,
      "description": "Wilgotność × 10 (%)"
    },
    // ... 9 more items
  ],
  "interpretedTempEXT": 23.0,
  "interpretedHumidity": 56.0,
  "interpretedTempCO": 0.0,
  "interpretedActuatorPos": 0
}
```

## User Workflow Example

### Scenario: User has HT73 sensor that doesn't respond

**Step 1: Open Diagnostics page**
- User sees "❌ Rozłączony" status

**Step 2: Click "🔍 Skanuj Slave ID (1-10)"**
- Wait 1-2 seconds
- Results show:
  ```
  Slave ID 5:
    - FC03 (Holding): ✅ OK
    - FC04 (Input):   ❌ Brak
  ```

**Step 3: Open Settings page**
- Go to "Konfiguracja Modbus - Zaawansowane"
- Set:
  - Typ rejestru: Holding Registers (FC 0x03)
  - Adres startowy: 0
  - Liczba rejestrów: 2
  - Mapowanie: HT73 (Reg0=Wilgotność, Reg1=Temperatura)
  - Dzielnik: ÷10

**Step 4: Save and restart**
- Click "💾 Zapisz ustawienia"
- ESP32 restarts

**Step 5: Verify on Dashboard**
- Temperature and humidity now display correctly
- Status shows "✅ Połączono z PLC"

**Step 6: Check Diagnostics**
- "Informacje diagnostyczne" confirms settings
- Register table shows correct values
- Logs show successful reads
