# Changelog

Wszystkie istotne zmiany w tym projekcie będą dokumentowane w tym pliku.

Format oparty na [Keep a Changelog](https://keepachangelog.com/pl/1.0.0/).

## [1.2.0] - 2026-01-28

### Dodano
- Zaawansowana konfiguracja Modbus (wybór FC03/FC04, adres startowy, liczba rejestrów)
- 4 tryby mapowania danych (HT73, HT73v2, PLC, Custom)
- 3 opcje dzielnika wartości (brak, ÷10, ÷100)
- Narzędzia diagnostyczne:
  - Skanowanie Slave ID (1-10)
  - Test FC03 (Holding Registers)
  - Test FC04 (Input Registers)
- Endpoint `/api/modbus-scan` - skanowanie urządzeń Modbus
- Endpoint `/api/modbus-test` - test konkretnego Function Code
- Rozszerzona tabela rejestrów (10 rejestrów z różnymi formatami)
- Informacje diagnostyczne na stronie Diagnostyka

### Zmieniono
- Rozszerzono strukturę `Config` o nowe pola:
  - `modbusUseInputRegisters`
  - `modbusStartRegister`
  - `modbusRegisterCount`
  - `modbusDataMapping`
  - `modbusDivider`
- Zaktualizowano endpoint `/api/config` (GET/POST) - obsługa nowych pól
- Rozszerzono stronę Ustawienia o sekcję "Zaawansowane"
- Rozszerzono stronę Diagnostyka o narzędzia testowe

### Naprawiono
- Dodano funkcję `processModbusData()` dla elastycznego przetwarzania danych
- Dodano funkcję `scanModbusSlaves()` dla automatycznego wykrywania urządzeń

## [1.1.0] - 2026-01-27

### Zmieniono
- Zmiana Serial2 → Serial (UART0) zgodnie z przykładem producenta ESP32C14
- Powrót do Holding Registers (FC03) zamiast Input Registers (FC04)
- Odczyt rejestrów 0-1 (wilgotność + temperatura)

### Usunięto
- Logi Serial Monitor (Serial zajęty przez RS485)

### Dodano
- Callbacks `preTransmission()` i `postTransmission()` dla DE/RE
- LED diagnostyczny na GPIO2 (miga gdy Modbus działa)

## [1.0.0] - 2026-01-26

### Dodano
- Pierwsza wersja systemu OPEC ESP32
- Podstawowa komunikacja Modbus RTU
- Interfejs WWW (Dashboard, Ustawienia, Krzywa grzewcza, Diagnostyka)
- Termostat z regulacją krzywej grzewczej
- Obsługa WiFi (STA + AP)
- Konfiguracja przez EEPROM
