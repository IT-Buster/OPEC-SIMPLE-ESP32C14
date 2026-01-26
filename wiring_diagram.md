# 🔌 Schemat połączeń - OPEC-SIMPLE-ESP32C14

## ⚡ Hardware
- **Mikrokontroler:** EleTechSup ES32C14 (ESP32-C3)
- **Interface RS485:** Wbudowany w płytkę ES32C14
- **Zasilanie:** 12V/24V DC lub AC 85-265V

---

## 📌 Połączenia RS485 → PLC Modbus

### ES32C14 → PLC Modbus RTU

| ES32C14 Port | ESP32 GPIO | Funkcja | PLC Modbus Terminal |
|--------------|------------|---------|---------------------|
| IO1          | GPIO 1     | A       | A (+)               |
| IO3          | GPIO 3     | B       | B (-)               |
| IO22         | GPIO 22    | DE/RE   | (wewnętrzna kontrola) |
| GND          | GND        | GND     | GND                 |

⚠️ **UWAGA:** ES32C14 ma **wbudowany konwerter RS485** - nie potrzebujesz zewnętrznego modułu MAX485!

**Konfiguracja DIP switch:**
- Upewnij się, że DIP switch SW1 jest ustawiony dla portu RS485 (sprawdź dokumentację płytki)

---

## 🔧 Schemat ASCII

```
┌─────────────────────────────────────────────────────┐
│       EleTechSup ES32C14 Board                    │
│  ┌──────────────────────────────────────────────┐  │
│  │  ESP32-C3 + RS485 Interface                  │  │
│  │                                               │  │
│  │  GPIO 1  (IO1)  ──┐                          │  │
│  │  GPIO 3  (IO3)  ──┼──► Built-in RS485        │  │
│  │  GPIO 22 (IO22) ──┘     Transceiver          │  │
│  │                            │                  │  │
│  └────────────────────────────┼──────────────────┘  │
│                                │                     │
│    P13 Connector (RS485):      │                     │
│    ┌────┬────┬────┐            │                     │
│    │ A  │ B  │GND │◄───────────┘                     │
│    └─┬──┴──┬─┴──┬─┘                                 │
└──────┼─────┼────┼───────────────────────────────────┘
       │     │    │
       A     B   GND
       │     │    │
   ┌───┴─────┴────┴───┐        ┌──────────────┐
   │   [120Ω]         │◄───────┤ Terminating  │
   │   A ←→ B         │        │  Resistor    │
   └───┬─────┬────┬───┘        └──────────────┘
       │     │    │
       A     B   GND
       │     │    │
   ┌───┴─────┴────┴────────────────────────────┐
   │         PLC Modbus RTU                    │
   │  (Unit ID: 2, 115200 baud, 8N1)           │
   │                                            │
   │  Registers:                                │
   │  - Reg 0: Actuator Position               │
   │  - Reg 4: External Temperature (x10)      │
   │  - Reg 5: CO Temperature (x10)            │
   └────────────────────────────────────────────┘
```

---

## 🔌 Zasilanie

### Opcja 1: DC 12V/24V
Podłącz zasilacz DC do złącza P1/P2:
```
Zasilacz 12V/24V DC
    (+) ──► P2 (+V)
    (-) ──► P2 (GND)
```

### Opcja 2: AC 85-265V (wbudowany zasilacz)
Podłącz bezpośrednio sieć AC:
```
Sieć 230V AC
    L  ──► AC Input (L)
    N  ──► AC Input (N)
    PE ──► Ground (opcjonalnie)
```

⚠️ **OSTRZEŻENIE:** 
- Instalację AC wykonuj zgodnie z lokalnymi przepisami elektrycznymi!
- Używaj bezpieczników/wyłączników nadprądowych!
- Zachowaj izolację galwaniczną!

---

## 🔗 Dodatkowe połączenia (opcjonalnie)

### Relay CH1 → Pompa obiegowa CO
```
ES32C14 RELAY1 (CH1)
    COM  ──► 230V L (Fase)
    NO   ──► Pompa L
    GND  ──► Pompa N
```

### Digital Input IN1 → Przycisk AUTO/MANUAL
```
Przycisk (NPN)
    (+24V) ──► IN1 (IO19)
    (GND)  ──► GND
```

---

## 📷 Zdjęcia ze schematu

![Wiring Diagram ES32C14](docs/images/es32c14-wiring.avif)

![Board Interfaces](docs/images/es32c14-interfaces.avif)

---

## 🛠️ Konfiguracja Modbus

W pliku `config.h` upewnij się, że parametry są zgodne z PLC:

```cpp
#define MODBUS_BAUDRATE    115200
#define MODBUS_UNIT_ID     2
#define MODBUS_PARITY      SERIAL_8N1
```

---

## 🔍 Testowanie połączenia

### Krok 1: Upload kodu do ESP32
```bash
Arduino IDE → Tools → Board → ESP32C3 Dev Module
            → Port → /dev/ttyUSB0 (lub COM3 na Windows)
            → Upload
```

### Krok 2: Monitor Serial (115200 baud)
```
Modbus Init OK
WiFi Connected: 192.168.1.100
Web Server Started: http://opec-esp32.local
Reading Modbus...
TempCO: 45.2°C, TempEXT: 8.5°C, Actuator: 65%
```

### Krok 3: Sprawdź WebUI
Otwórz przeglądarkę: `http://opec-esp32.local`

---

## ❗ Troubleshooting

### Błąd: "Modbus Timeout"
- ✅ Sprawdź okablowanie A/B (spróbuj zamienić)
- ✅ Sprawdź Unit ID w PLC (musi być `2`)
- ✅ Sprawdź baudrate w PLC (musi być `115200`)
- ✅ Dodaj rezystor terminujący 120Ω między A-B

### Błąd: "RS485 Read Error"
- ✅ Upewnij się, że GPIO 1, 3, 22 są poprawnie zmapowane
- ✅ Sprawdź zasilanie GND (wspólna masa!)
- ✅ Zmierz napięcie na A/B (powinno być ~2.5V relative)

### Połączenie WiFi nie działa
- ✅ Sprawdź SSID/hasło w WebUI (tryb AP: `OPEC-ESP32-XXXXX`)
- ✅ Reset ustawień: przytrzymaj BOOT 5s przy starcie

---

## 📝 Licencja

Schemat dostępny na licencji MIT - zobacz LICENSE w głównym katalogu projektu.

**Autor:** IT-Buster  
**Data:** 2026
