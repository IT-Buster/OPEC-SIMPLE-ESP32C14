# 🔧 Specyfikacja Hardware - ElecTechSup ES32C14

## Przegląd płytki

ElecTechSup ES32C14 to rozbudowana płyta rozwojowa oparta na **ESP32-C3** z wbudowanymi interfejsami przemysłowymi:

- ✅ **RS485** (Modbus RTU)
- ✅ **4x Relay 10A** (sterowanie obciążeniami AC/DC)
- ✅ **4x Digital Input** (NPN, 24V)
- ✅ **2x Analog Input 0-10V**
- ✅ **2x Analog Input 0-20mA**
- ✅ **2x Analog Output 0-10V / 0-20mA** (multipleksowane)
- ✅ Zasilanie: **12V/24V DC** lub **AC 85-265V**

---

## 📌 Mapowanie pinów

### Porty RS485 (Modbus RTU)
| Złącze ES32C14 | ESP32 GPIO | Funkcja RS485 | Opis |
|----------------|------------|---------------|------|
| IO1            | GPIO 1     | DI (TX)       | Data Input (transmisja do RS485) |
| IO3            | GPIO 3     | RO (RX)       | Receive Output (odbiór z RS485) |
| IO22           | GPIO 22    | DE + RE       | Direction Enable (kontrola kierunku) |

**Połączenie z PLC Modbus:**
- IO1 (GPIO1) → Moduł RS485 → A/B do PLC
- IO3 (GPIO3) → Moduł RS485 → A/B z PLC
- IO22 (GPIO22) → Kontrola DE/RE modułu RS485

---

### Wyjścia Relay (10A, 250V AC)
| Kanał  | ESP32 GPIO | Connector | Maksymalne obciążenie |
|--------|------------|-----------|------------------------|
| CH1    | IO27       | RELAY1    | 10A @ 250V AC / 30V DC |
| CH2    | IO14       | RELAY2    | 10A @ 250V AC / 30V DC |
| CH3    | IO12       | RELAY3    | 10A @ 250V AC / 30V DC |
| CH4    | IO13       | RELAY4    | 10A @ 250V AC / 30V DC |

**Przykładowe zastosowania:**
- Sterowanie pompą obiegową CO
- Zawory trójdrożne
- Sygnalizacja (lampki LED, buzzer)

---

### Wejścia cyfrowe NPN (24V)
| Wejście | ESP32 GPIO | Typ       | Napięcie |
|---------|------------|-----------|----------|
| IN1     | IO19       | NPN       | 0-24V DC |
| IN2     | IO18       | NPN       | 0-24V DC |
| IN3     | IO5        | NPN       | 0-24V DC |
| IN4     | IO17       | NPN       | 0-24V DC |

**Przykładowe zastosowania:**
- Przycisk RESET
- Czujnik otwarcia okna
- Czujnik PIR (obecność)
- Przełącznik trybu

---

### Wejścia analogowe
| Wejście | ESP32 GPIO | Zakres     | Rozdzielczość |
|---------|------------|------------|---------------|
| Vi1     | IO36       | 0-10V      | 12-bit ADC    |
| Vi2     | IO39       | 0-10V      | 12-bit ADC    |
| Ii1     | IO34       | 0-20mA     | 12-bit ADC    |
| Ii2     | IO35       | 0-20mA     | 12-bit ADC    |

**Przykładowe zastosowania:**
- Czujnik temperatury 0-10V (alternatywa dla Modbus)
- Czujnik ciśnienia 4-20mA
- Natężenie światła (LUX)

---

### Wyjścia analogowe (MULTIPLEKSOWANE!)
| Wyjście    | ESP32 GPIO | Zakres      | Wybór trybu       |
|------------|------------|-------------|-------------------|
| Vo1 / Io1  | IO25/IO26  | 0-10V / 0-20mA | DIP switch SW1 |
| Vo2 / Io2  | IO25/IO26  | 0-10V / 0-20mA | DIP switch SW1 |

⚠️ **UWAGA:** Tylko **jeden tryb** (Vo lub Io) może być aktywny jednocześnie!
- Ustaw DIP switch SW1 zgodnie z potrzebami:
  - Vo1/Vo2: Wyjście napięciowe 0-10V
  - Io1/Io2: Wyjście prądowe 0-20mA

**Przykładowe zastosowania:**
- Sterowanie siłownikiem 0-10V (HVAC)
- Sterowanie falownikiem 4-20mA

---

### Wolne porty GPIO (Free Port)
Dostępne do wykorzystania w projekcie:
```
IO33, IO32, IO15, IO2, IO0, IO4, IO16, IO21, IO23
```

**Przykładowe zastosowania:**
- Czujniki DHT22 (temperatura/wilgotność)
- Wyświetlacz OLED I2C
- Dodatkowe LED statusu
- Czujnik ruchu PIR

---

## 🔌 Zasilanie

### Opcja 1: DC 12V/24V (Connector P1/P2)
| Pin | Funkcja |
|-----|---------|
| +V  | 12V lub 24V DC |
| GND | Ground (masa) |

### Opcja 2: AC 85-265V (wbudowany zasilacz)
Płytka posiada wbudowany zasilacz AC/DC umożliwiający zasilanie bezpośrednio z sieci 230V AC.

⚠️ **Ostrzeżenie:** Wysokie napięcie! Instalację AC powierzaj wykwalifikowanemu elektrykowi!

---

## 📷 Zdjęcia i diagramy

### Pinout płytki
![ES32C14 Pinout](images/es32c14-pinout.jpg)

### Opis interfejsów
![ES32C14 Interfaces](images/es32c14-interfaces.jpg)

### Wiring Diagram
![ES32C14 Wiring](images/es32c14-wiring.jpg)

### Konfiguracja DIP Switch
![ES32C14 DIP Switch](images/es32c14-dip-switch.jpg)

---

## 🛠️ Zastosowanie w projekcie OPEC

### Minimalna konfiguracja (tylko Modbus + Termostat):
- **RS485:** IO1, IO3, IO22 → Komunikacja z PLC
- **Relay CH1:** IO27 → Sterowanie pompą CO (opcjonalnie)
- **LED Status:** IO2 → Sygnalizacja pracy termostatu

### Rozszerzona konfiguracja (z dodatkowymi czujnikami):
- **RS485:** Modbus RTU do PLC
- **Relay CH1-CH4:** Pompa, zawór, wentylator, alarm
- **IN1-IN2:** Przyciski trybu (AUTO/MANUAL)
- **Vi1:** Czujnik temperatury zewnętrznej 0-10V (backup dla Modbus)
- **Ii1:** Czujnik ciśnienia w instalacji CO (4-20mA)

---

## 🔗 Linki

- **Dokumentacja producenta:** [ElecTechSup ES32C14](https://www.electechsup.com/)
- **Datasheet ESP32-C3:** [Espressif ESP32-C3 Datasheet](https://www.espressif.com/en/products/socs/esp32-c3)
- **Kod Arduino:** [OPEC-SIMPLE-ESP32C14](https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14)

---

## 📝 Licencja

Dokumentacja dostępna na licencji MIT - zobacz plik LICENSE w głównym katalogu projektu.
