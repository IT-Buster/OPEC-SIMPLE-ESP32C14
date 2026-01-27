# OPEC-SIMPLE-ESP32C14

🔥 **Sterownik systemu grzewczego dla ESP32-C3 z komunikacją Modbus RTU**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino](https://img.shields.io/badge/Arduino-IDE-00979D?logo=arduino)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/ESP32-C3-E7352C?logo=espressif)](https://www.espressif.com/)

> Projekt Arduino dla płytki **EletechSup ES32C14 (ESP32-C3)** do sterowania systemem grzewczym przez Modbus RTU RS485. Konwersja projektu [OPEC-SIMPLE](https://github.com/IT-Buster/OPEC-SIMPLE) (Node-RED) na natywny kod C++.

[🇬🇧 English version](README_EN.md)

---

## 📖 Spis treści

- [Opis projektu](#-opis-projektu)
- [Główne funkcje](#-główne-funkcje)
- [Wymagania](#-wymagania)
- [Instalacja](#-instalacja)
- [Konfiguracja początkowa](#-konfiguracja-początkowa)
- [Używanie](#-używanie)
- [API Documentation](#-api-documentation)
- [Schemat elektryczny](#-schemat-elektryczny)
- [Troubleshooting](#-troubleshooting)
- [Rozwój projektu](#-rozwój-projektu)
- [Licencja](#-licencja)

---

## 🎯 Opis projektu

**OPEC-SIMPLE-ESP32C14** to kompletny system sterowania ogrzewaniem oparty na mikrokontrolerze ESP32-C3, który komunikuje się z PLC przez Modbus RTU (RS485).

### Co robi system?

- 📊 **Odczytuje dane** z PLC przez Modbus RTU (temperatura CO, temperatura zewnętrzna, pozycja siłownika)
- 🌡️ **Kontroluje temperaturę** za pomocą termostatu z algorytmem histerezy
- 📈 **Wykorzystuje krzywą grzewczą** do automatycznego obliczania temperatury zadanej w zależności od temperatury zewnętrznej
- 🌐 **Oferuje interfejs webowy** do monitorowania i konfiguracji systemu
- 📡 **Działa w trybie WiFi** AP (punkt dostępu) lub STA (klient sieci)
- 💾 **Zapisuje konfigurację** w pamięci Flash (Preferences)

### Dlaczego ESP32 zamiast Node-RED?

- ✅ Autonomiczny sterownik bez zależności od serwera
- ✅ Niskie zużycie energii
- ✅ Kompaktowa budowa (pojedynczy moduł)
- ✅ Szybsze działanie i większa niezawodność
- ✅ Niższy koszt całkowity systemu

---

## ⭐ Główne funkcje

### 1. Komunikacja Modbus RTU (RS485)
- Wykorzystanie wbudowanego portu UART w ES32C14
- Parametry: 115200 baud, 8N1, Unit ID: 2 (konfigurowalne)
- Odczyt rejestrów: pozycja siłownika, temperatura zewnętrzna, temperatura CO
- Zapis: sterowanie siłownikiem (0-100%)
- Automatyczny retry przy błędach (max 3 próby)
- Odczyt co 15 sekund

### 2. Termostat z histerezą
- Porównanie temperatury CO z temperaturą zadaną
- Histereza ±1°C (konfigurowalna)
- Algorytm:
  - TempCO < (Nastawa - 1°C) → otwórz siłownik (100%)
  - TempCO > (Nastawa + 1°C) → zamknij siłownik (0%)
  - W zakresie histerezy → utrzymuj stan
- Tryb AUTO/MANUAL

### 3. Krzywa grzewcza
- Automatyczne obliczanie temperatury zadanej CO na podstawie temp. zewnętrznej
- Interpolacja liniowa między 9 punktami krzywej
- Domyślna krzywa (edytowalna przez WebUI):

| Temp. Zewnętrzna | Temp. CO |
|------------------|----------|
| -20°C            | 65°C     |
| -15°C            | 60°C     |
| -10°C            | 55°C     |
| -5°C             | 50°C     |
| 0°C              | 45°C     |
| 5°C              | 40°C     |
| 10°C             | 35°C     |
| 15°C             | 30°C     |
| 20°C             | 25°C     |

### 4. Interfejs webowy (WebUI)
**Dashboard:**
- Wyświetlanie w czasie rzeczywistym:
  - Temperatura CO, temperatura zewnętrzna, temperatura zadana
  - Pozycja siłownika (0-100%)
  - Status termostatu (ON/OFF)
  - Status połączenia WiFi i Modbus
  - Czas pracy, wolna pamięć
- Auto-odświeżanie co 5 sekund
- Przełącznik trybu AUTO/MANUAL

**Krzywa grzewcza:**
- Tabela z edytowalnymi punktami krzywej (9 punktów)
- Przycisk "Zapisz" → zapis do Flash
- Przycisk "Przywróć domyślne"

**Ustawienia:**
- Konfiguracja WiFi (SSID, hasło)
- Parametry Modbus (adres Unit ID, baudrate)
- Histereza termostatu (0.1 - 5.0°C)
- Reset do ustawień fabrycznych

### 5. WiFi i sieć
- **Tryb AP:** Przy pierwszym uruchomieniu lub braku zapisanej sieci
  - SSID: `OPEC-ESP32-XXXX` (gdzie XXXX = chip ID)
  - Hasło: `opec1234`
  - IP: `192.168.4.1`
- **Tryb STA:** Połączenie z domową siecią WiFi
- **mDNS:** Dostęp przez `http://opec-esp32.local`

---

## 🔧 Specyfikacja Hardware

Projekt został zaprojektowany dla płytki **EletechSup ES32C14** (ESP32-C3) z wbudowanymi interfejsami:

### Kluczowe cechy płytki:
- ✅ **ESP32-C3** (RISC-V, WiFi, Bluetooth 5.0)
- ✅ **RS485 Modbus RTU** (IO1, IO3, IO22)
- ✅ **4x Relay 10A** (sterowanie pompą, zaworami)
- ✅ **4x Digital Input NPN** (przyciski, czujniki)
- ✅ **Analog I/O** (0-10V, 0-20mA)
- ✅ Zasilanie **12V/24V DC** lub **AC 85-265V**

### 📷 Dokumentacja płytki:

![ES32C14 Board](docs/images/es32c14-interfaces.avif)

**Pełna specyfikacja:** Zobacz [docs/ES32C14_HARDWARE.md](docs/ES32C14_HARDWARE.md)

**English version:** See [docs/ES32C14_HARDWARE_EN.md](docs/ES32C14_HARDWARE_EN.md)

---

### Mapowanie pinów RS485 (Modbus RTU):

| Funkcja RS485 | ESP32 GPIO | ES32C14 Port |
|---------------|------------|--------------|
| TX (DI)       | GPIO 1     | IO1          |
| RX (RO)       | GPIO 3     | IO3          |
| DE/RE         | GPIO 22    | IO22         |

**Schemat okablowania:**
```
ES32C14 (IO1, IO3, IO22) → Wbudowany RS485 → A/B → PLC Modbus
                                                ↓
                                        Rezystor 120Ω (opcjonalny)
```

![Wiring Diagram](docs/images/es32c14-wiring.avif)

### ⚠️ Important Note for ESP32-D0WD variant

If your ES32C14 board has **ESP32-D0WD-V3** chip (instead of ESP32-C3):

- RS485 uses **Serial (UART0)** on GPIO 1, 3, 22
- **Serial Monitor will NOT work** during Modbus communication (shared pins with USB)
- Use **WebUI Dashboard** (`http://opec-esp32.local`) to monitor system status
- Debugging: Use Serial1 (GPIO 9 TX, GPIO 10 RX) with external USB-Serial adapter

## ⚠️ UWAGA - Serial Monitor NIE działa!

**Serial (UART0) jest używany przez RS485 zgodnie z projektem płyty ES32C14.**

Po wgraniu kodu:
- ✅ **Serial Monitor przestanie działać** (Serial zajęty przez RS485)
- ✅ **Czujnik HT73 zacznie odpowiadać** (domyślnie: Slave ID=5, Baudrate=9600)
- ✅ **Monitorowanie systemu tylko przez interfejs WWW:**
  - Dashboard: `http://opec-esp32.local` lub `http://192.168.4.1` (tryb AP)
  - Strona Dashboard - temperatura i wilgotność w czasie rzeczywistym
  - Strona Diagnostyka - surowe rejestry Modbus
  - Strona Ustawienia - konfiguracja Modbus (Slave ID, Baudrate)

**Aby debugować przez Serial Monitor, odłącz czujnik RS485 i zmień kod aby używał Serial2.**

**Wiring:**
```
ES32C14 RS485 Port → PLC Modbus
A (+)              → A
B (-)              → B  
GND                → GND
```

**PLC Configuration:**
- Slave ID: 2 (default, configurable in PLC)
- Baudrate: 115200
- Parity: None (8N1)

---

## 📋 Wymagania

### Hardware
- **Mikrokontroler:** EletechSup ES32C14 (ESP32-C3) lub kompatybilny
- **Interface RS485:** Wbudowany w płytkę ES32C14
- **PLC Modbus:** Urządzenie z interfejsem Modbus RTU
- **Zasilanie:** 12V/24V DC lub AC 85-265V
- **Kable:** Skrętka ekranowana do RS485

### Software
- **Arduino IDE:** 2.0 lub nowsze
- **Board Support:** ESP32 by Espressif Systems
- **Biblioteki:** (patrz [libraries.txt](libraries.txt))
  - ModbusMaster (v2.0.1+)
  - ESPAsyncWebServer
  - AsyncTCP
  - ArduinoJson (v6.21.0+)
  - Preferences (wbudowana)
  - WiFi (wbudowana)
  - ESPmDNS (wbudowana)

---

## 🚀 Instalacja

### Krok 1: Instalacja Arduino IDE
1. Pobierz [Arduino IDE](https://www.arduino.cc/en/software) (wersja 2.0+)
2. Zainstaluj i uruchom

### Krok 2: Dodanie obsługi ESP32
1. Otwórz **File → Preferences**
2. W polu "Additional Board Manager URLs" dodaj:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Kliknij **OK**
4. Otwórz **Tools → Board → Boards Manager**
5. Wyszukaj "ESP32" i zainstaluj "**ESP32 by Espressif Systems**"

### Krok 3: Instalacja bibliotek
1. Otwórz **Tools → Manage Libraries** (Ctrl+Shift+I)
2. Zainstaluj następujące biblioteki:
   - **ModbusMaster** by Doc Walker
   - **ArduinoJson** by Benoit Blanchon (v6.x)

3. Zainstaluj biblioteki z GitHub (ręcznie):
   
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

### Krok 4: Pobranie projektu
```bash
git clone https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14.git
cd OPEC-SIMPLE-ESP32C14
```

Alternatywnie: Pobierz ZIP z GitHub → **Code → Download ZIP**

### Krok 5: Konfiguracja Arduino IDE
1. Otwórz plik `OPEC-SIMPLE-ESP32C14.ino` w Arduino IDE
2. Wybierz board:
   - **Tools → Board → ESP32 Arduino → ESP32C3 Dev Module**
3. Ustaw parametry:
   - **Upload Speed:** 921600
   - **CPU Frequency:** 160MHz
   - **Flash Size:** 4MB (32Mb)
   - **Partition Scheme:** Default 4MB (bez SPIFFS)
   - **Core Debug Level:** None (dla release) lub Info (dla debugowania)

### Krok 6: Upload kodu
1. Podłącz ESP32 przez USB
2. Wybierz odpowiedni port: **Tools → Port → COMX** (Windows) lub **/dev/ttyUSBX** (Linux)
3. Kliknij **Upload** (Ctrl+U)
4. Poczekaj na zakończenie (ok. 30-60 sekund)
5. **Gotowe!** WebUI jest wbudowane w kod - nie potrzebujesz uploadować plików SPIFFS

### Krok 7: Podłączenie hardware
Połącz ESP32 z modułem RS485 zgodnie ze schematem w [wiring_diagram.md](wiring_diagram.md)

---

## ⚙️ Konfiguracja początkowa

### Pierwsze uruchomienie

1. **Zasilanie:** Podłącz ESP32 do zasilania (USB lub 5V zewnętrzne)

2. **Tryb AP:** ESP32 automatycznie utworzy punkt dostępu WiFi:
   - SSID: `OPEC-ESP32-XXXX` (gdzie XXXX to ID chipu)
   - Hasło: `opec1234`
   - IP: `192.168.4.1`

3. **Połączenie:** Połącz się z WiFi ESP32 używając telefonu lub komputera

4. **WebUI:** Otwórz przeglądarkę i wejdź na `http://192.168.4.1`

### Konfiguracja WiFi domowego

1. Przejdź do **Ustawienia** w menu WebUI
2. Wprowadź **SSID** i **hasło** swojej sieci WiFi
3. Kliknij **Zapisz ustawienia**
4. ESP32 zrestartuje się i połączy z Twoją siecią

### Konfiguracja Modbus

1. W sekcji **Ustawienia** ustaw:
   - **Unit ID:** Adres Modbus PLC (domyślnie: 2)
   - **Baudrate:** Prędkość transmisji (domyślnie: 115200)
2. Kliknij **Zapisz ustawienia**
3. Zrestartuj ESP32 (odłącz i podłącz zasilanie)

### Ustawienie krzywej grzewczej

1. Przejdź do **Krzywa** w menu WebUI
2. Dostosuj punkty krzywej według potrzeb
3. Kliknij **Zapisz krzywą**

---

## 💡 Używanie

### Dostęp do WebUI

Po skonfigurowaniu WiFi, dostęp do sterownika:

- **mDNS:** `http://opec-esp32.local`
- **IP:** Sprawdź IP w routerze lub Serial Monitor

### Dashboard

Dashboard wyświetla w czasie rzeczywistym:
- 🌡️ Temperatura CO
- 🌡️ Temperatura zewnętrzna
- 🎯 Temperatura zadana (obliczona z krzywej)
- 🔧 Pozycja siłownika (0-100%)
- ✅ Status termostatu (ON/OFF)
- 📡 Status Modbus
- 📶 Jakość WiFi
- ⏱️ Czas pracy
- 💾 Wolna pamięć

**Auto-odświeżanie:** Co 5 sekund

### Sterowanie termostatem

**Tryb AUTO:**
- Termostat aktywny
- Temperatura zadana obliczana z krzywej grzewczej
- Automatyczne sterowanie siłownikiem

**Tryb MANUAL:**
- Termostat wyłączony
- Siłownik sterowany ręcznie (przez WebUI lub zewnętrznie)

**Przełączanie:** Użyj przełącznika na Dashboard

### Edycja krzywej grzewczej

1. Przejdź do **Krzywa**
2. Edytuj wartości w tabeli
3. Kliknij **Zapisz krzywą**
4. Nowa krzywa zostanie zastosowana natychmiast

**Przywrócenie domyślnej krzywej:**
- Kliknij **Przywróć domyślne**
- Potwierdź
- Kliknij **Zapisz krzywą**

### Zmiana ustawień

1. Przejdź do **Ustawienia**
2. Zmień wybrane parametry:
   - WiFi SSID/hasło
   - Modbus Unit ID/Baudrate
   - Histereza termostatu
3. Kliknij **Zapisz ustawienia**
4. Zrestartuj ESP32 jeśli zmieniałeś WiFi lub Modbus

### Reset do ustawień fabrycznych

1. **Przez WebUI:**
   - Przejdź do **Ustawienia**
   - Kliknij **Reset do fabrycznych**
   - Potwierdź
   - ESP32 zrestartuje się

2. **Przez przycisk BOOT:**
   - Przytrzymaj przycisk BOOT przez 5 sekund podczas startu
   - ESP32 zresetuje się do ustawień domyślnych

---

## 🔌 API Documentation

System oferuje REST API do integracji z zewnętrznymi aplikacjami.

### GET /api/status

Zwraca aktualny stan systemu.

**Przykładowa odpowiedź:**
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

**Przykład curl:**
```bash
curl http://opec-esp32.local/api/status
```

### GET /api/curve

Zwraca aktualną krzywą grzewczą.

**Przykładowa odpowiedź:**
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

Aktualizuje krzywą grzewczą.

**Przykładowy request:**
```bash
curl -X POST http://opec-esp32.local/api/curve \
  -H "Content-Type: application/json" \
  -d '{"curve":[[-20,65],[-15,60],[-10,55],[-5,50],[0,45],[5,40],[10,35],[15,30],[20,25]]}'
```

### POST /api/thermostat

Włącza/wyłącza termostat.

**Przykładowy request:**
```bash
# Włącz termostat
curl -X POST http://opec-esp32.local/api/thermostat \
  -H "Content-Type: application/json" \
  -d '{"active":true}'

# Wyłącz termostat
curl -X POST http://opec-esp32.local/api/thermostat \
  -H "Content-Type: application/json" \
  -d '{"active":false}'
```

### GET /api/settings

Zwraca aktualne ustawienia (bez hasła WiFi).

**Przykładowa odpowiedź:**
```json
{
  "wifiSSID": "MojaWiFi",
  "modbusUnitID": 2,
  "modbusBaudrate": 115200,
  "hysteresis": 1.0,
  "thermostatActive": true
}
```

### POST /api/settings

Aktualizuje ustawienia systemu.

**Przykładowy request:**
```bash
curl -X POST http://opec-esp32.local/api/settings \
  -H "Content-Type: application/json" \
  -d '{"wifiSSID":"NowaWiFi","wifiPassword":"haslo123","hysteresis":1.5}'
```

### POST /api/reset

Resetuje ustawienia do wartości fabrycznych i restartuje urządzenie.

**Przykładowy request:**
```bash
curl -X POST http://opec-esp32.local/api/reset
```

---

## ⚡ Schemat elektryczny

Szczegółowy schemat połączeń znajduje się w pliku [wiring_diagram.md](wiring_diagram.md).

### Piny ESP32-C3 (ES32C14)

| Pin GPIO | Funkcja | Opis                    |
|----------|---------|-------------------------|
| GPIO 1   | TX      | UART TX (RS485 DI)      |
| GPIO 3   | RX      | UART RX (RS485 RO)      |
| GPIO 22  | DE/RE   | Direction Enable RS485  |
| 12-24V   | VCC     | Zasilanie               |
| GND      | GND     | Masa                    |

### Połączenie z wbudowanym RS485 → PLC Modbus

```
ES32C14 Built-in RS485 → PLC
A (+)                   → A
B (-)                   → B
GND                     → GND
```

**⚠️ Ważne:**
- ES32C14 ma **wbudowany konwerter RS485** - nie potrzebujesz zewnętrznego modułu MAX485
- Dodaj rezystor terminujący 120Ω między A i B jeśli ESP32 jest na końcu linii
- Użyj skrętki ekranowanej dla odporności na zakłócenia
- Upewnij się, że wszystkie urządzenia mają wspólną masę (GND)

---

## 🔧 Troubleshooting

### Brak połączenia WiFi

**Problem:** ESP32 nie łączy się z siecią WiFi

**Rozwiązanie:**
1. Sprawdź SSID i hasło w ustawieniach
2. Upewnij się, że sieć WiFi jest w zasięgu
3. Sprawdź czy router obsługuje WiFi 2.4GHz (ESP32 nie obsługuje 5GHz)
4. Spróbuj zresetować ustawienia do fabrycznych
5. Sprawdź Serial Monitor (115200 baud) dla szczegółowych logów

### Błędy Modbus

**Problem:** Brak komunikacji z PLC, błędy odczytu

**Rozwiązanie:**
1. Sprawdź okablowanie RS485 (A, B, GND)
2. Spróbuj zamienić przewody A ↔ B
3. Sprawdź adres Unit ID w ustawieniach (czy zgadza się z PLC)
4. Sprawdź baudrate (czy zgadza się z PLC)
5. Dodaj rezystor terminujący 120Ω między A i B
6. Sprawdź zasilanie modułu RS485
7. Użyj Modbus Scanner do testowania połączenia
8. Sprawdź logi w Serial Monitor

### WebUI nie ładuje się

**Problem:** Strona nie wyświetla się lub zwraca błąd 404

**Rozwiązanie:**
1. Upewnij się, że wgrałeś najnowszą wersję kodu (WebUI jest wbudowane w kod od wersji 1.1+)
2. Sprawdź Serial Monitor - powinno być "Strony HTML wbudowane w kod (bez SPIFFS)"
3. Spróbuj użyć adresu IP zamiast mDNS
4. Wyczyść cache przeglądarki (Ctrl+F5)
5. Jeśli używasz starszej wersji kodu, zaktualizuj do najnowszej

### Termostat nie działa

**Problem:** Siłownik nie reaguje na zmiany temperatury

**Rozwiązanie:**
1. Sprawdź czy termostat jest włączony (tryb AUTO)
2. Sprawdź połączenie Modbus (czy dane są odczytywane)
3. Sprawdź krzywą grzewczą (czy temperatury są prawidłowe)
4. Sprawdź histerezę (czy nie jest zbyt duża)
5. Sprawdź logi w Serial Monitor dla algorytmu termostatu
6. Sprawdź zakres temperatury zewnętrznej (czy mieści się w krzywej)

### System zawiesza się

**Problem:** ESP32 resetuje się lub zawiesza

**Rozwiązanie:**
1. Sprawdź zasilanie (min. 500mA, stabilizowane 5V)
2. Sprawdź logi dla Stack Overflow lub Watchdog Reset
3. Zwiększ opóźnienia w kodzie (jeśli modyfikowałeś)
4. Sprawdź temperaturę ESP32 (nie powinna przekraczać 80°C)
5. Wgraj kod ponownie z Debug Level ustawionym na "Verbose"

### mDNS nie działa

**Problem:** Nie można uzyskać dostępu przez `opec-esp32.local`

**Rozwiązanie:**
1. Upewnij się, że jesteś w tej samej sieci co ESP32
2. Sprawdź czy Twój system obsługuje mDNS:
   - **Windows:** Zainstaluj Bonjour Print Services
   - **Linux:** Zainstaluj Avahi
   - **macOS:** Działa natywnie
3. Użyj bezpośredniego adresu IP zamiast mDNS
4. Sprawdź Serial Monitor dla statusu mDNS

---

## 🚀 Rozwój projektu

### Planowane funkcje

- [ ] **MQTT:** Integracja z Home Assistant, Domoticz
- [ ] **ThingSpeak:** Wysyłanie danych do chmury
- [ ] **OTA Update:** Aktualizacja firmware przez WiFi
- [ ] **Harmonogram:** Planowanie temperatur na różne pory dnia
- [ ] **Multi-zone:** Obsługa wielu stref grzewczych
- [ ] **Mobile App:** Dedykowana aplikacja mobilna
- [ ] **Powiadomienia:** Email/SMS przy błędach
- [ ] **Wykres:** Wizualizacja historii temperatur

### Jak kontrybuować

Chętnie przyjmujemy Pull Requesty! 🎉

1. Fork repozytorium
2. Utwórz branch dla swojej funkcji (`git checkout -b feature/NowaFunkcja`)
3. Commit zmian (`git commit -m 'Dodaj NowaFunkcja'`)
4. Push do brancha (`git push origin feature/NowaFunkcja`)
5. Otwórz Pull Request

### Zgłaszanie błędów

Znalazłeś bug? Zgłoś go przez [GitHub Issues](https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14/issues)

Pamiętaj o załączeniu:
- Wersji Arduino IDE i ESP32 Core
- Logów z Serial Monitor
- Opisu kroków do reprodukcji problemu

---

## 📄 Licencja

Ten projekt jest udostępniony na licencji **MIT License**.

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

## 👤 Autor

**IT-Buster**
- GitHub: [@IT-Buster](https://github.com/IT-Buster)
- Projekt bazuje na: [OPEC-SIMPLE](https://github.com/IT-Buster/OPEC-SIMPLE) (Node-RED)

---

## 🙏 Podziękowania

- [ModbusMaster](https://github.com/4-20ma/ModbusMaster) by Doc Walker
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) by me-no-dev
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) by Benoit Blanchon
- Społeczność ESP32 i Arduino

---

**⭐ Jeśli projekt Ci się podoba, daj gwiazdkę na GitHubie! ⭐**
