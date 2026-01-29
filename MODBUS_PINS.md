# 📌 Piny Modbus RS485 dla ESP32C14

## Pytanie: Jakie piny są wykorzystane w ESP32 do komunikacji z Modbus?

### ✅ Odpowiedź:

Płytka **EletechSup ES32C14** wykorzystuje następujące piny GPIO ESP32-C3 do komunikacji Modbus RTU przez RS485:

| GPIO Pin | Funkcja RS485 | Opis |
|----------|---------------|------|
| **GPIO1** | **TX** (Transmit) | Wysyłanie danych do urządzenia Modbus |
| **GPIO3** | **RX** (Receive) | Odbieranie danych z urządzenia Modbus |
| **GPIO22** | **DE/RE** (Direction Enable) | Kontrola kierunku transmisji (TX/RX) |

---

## 🔧 Szczegóły techniczne

### 1. UART0 (Serial)
- ES32C14 wykorzystuje **UART0** (obiekt `Serial` w Arduino) do komunikacji RS485
- GPIO1 (TX) i GPIO3 (RX) są **domyślnymi pinami UART0** w ESP32-C3
- Te piny są **wbudowane w hardware** i nie można ich zmienić

### 2. Pin DE/RE (GPIO22)
- **DE** (Driver Enable) - włącza nadajnik RS485
- **RE** (Receiver Enable) - włącza odbiornik RS485  
- GPIO22 steruje trybem pracy:
  - `HIGH` → Tryb nadawania (TX)
  - `LOW` → Tryb odbierania (RX)

### 3. Wbudowany transceiver RS485
ES32C14 ma **wbudowany konwerter RS485** podłączony do tych pinów - **nie potrzebujesz zewnętrznego modułu MAX485!**

---

## 💻 Kod źródłowy

### Definicja w `config.h`:
```cpp
// RS485 używa Serial (UART0) - domyślne piny TX(GPIO1)/RX(GPIO3)
// Tylko DE/RE wymaga definicji
#define RS485_DE_RE_PIN  22  // IO22 → DE+RE (Direction Enable)
```

### Inicjalizacja w `modbus_handler.cpp`:
```cpp
void setupModbus() {
  // UART0 (Serial) z pinami GPIO1/GPIO3
  Serial.begin(config.modbusBaudrate, SERIAL_8N1);
  Serial.setTimeout(5);
  
  // Konfiguracja pinu DE/RE (GPIO 22)
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW); // Tryb RX (odbiór)
  
  // Inicjalizacja biblioteki ModbusMaster
  modbus.begin(config.modbusUnitID, Serial);
  
  // Callback dla przełączania TX/RX
  modbus.preTransmission(preTransmission);
  modbus.postTransmission(postTransmission);
}
```

---

## ⚠️ WAŻNE UWAGI

### 1. Serial Monitor NIE będzie działał
- **GPIO1/GPIO3** są używane zarówno przez USB (Serial Monitor) jak i RS485
- Podczas komunikacji Modbus **Serial Monitor nie będzie dostępny**
- Do monitorowania systemu używaj **interfejsu WWW**: `http://opec-esp32.local`

### 2. Nie można zmienić pinów
- GPIO1 (TX) i GPIO3 (RX) są **sprzętowo przypisane do UART0**
- To są jedyne piny obsługujące komunikację szeregową w ESP32-C3
- ES32C14 wykorzystuje tę konfigurację sprzętową

### 3. Współdzielenie z USB
- Te same piny obsługują komunikację USB (programowanie) i RS485
- Podczas normalnej pracy wybierz jedno: **programowanie** LUB **Modbus**
- Nie można używać obu jednocześnie

---

## 🔌 Podłączenie fizyczne

### Złącze P13 (RS485) na płytce ES32C14:

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
  Do urządzenia Modbus (PLC, HT73, etc.)
```

### Połączenie z urządzeniem Modbus:

| ES32C14 P13 | → | Urządzenie Modbus |
|-------------|---|-------------------|
| A           | → | A (lub +, Data+)  |
| B           | → | B (lub -, Data-)  |
| GND         | → | GND (masa)        |

---

## 📚 Dodatkowe zasoby

- **Schemat połączeń:** [wiring_diagram.md](wiring_diagram.md)
- **Konfiguracja Modbus:** [ADVANCED_MODBUS_CONFIG.md](ADVANCED_MODBUS_CONFIG.md)
- **Plik nagłówkowy:** [config.h](config.h) (linie 23-41)
- **Implementacja:** [modbus_handler.cpp](modbus_handler.cpp)

---

## 🎯 Podsumowanie

**Piny wykorzystane do komunikacji Modbus RS485:**
- ✅ **GPIO1** - TX (transmisja danych)
- ✅ **GPIO3** - RX (odbiór danych)  
- ✅ **GPIO22** - DE/RE (kontrola kierunku)

**Platforma hardware:**
- Płytka: **EletechSup ES32C14**
- Chip: **ESP32-C3**
- Interface: **Wbudowany RS485 transceiver**
- UART: **UART0 (Serial)**

---

*Dokument utworzony: 2026-01-29*  
*Autor: IT-Buster*  
*Wersja firmware: 1.2.0*
