# Zaawansowana konfiguracja Modbus - Instrukcja

## 🎯 Cel

Ten dokument opisuje nowe funkcje zaawansowanej konfiguracji Modbus dodane w celu rozwiązania problemów z czujnikiem HT73 i innymi urządzeniami Modbus.

## ✨ Nowe funkcje

### 1. Rozszerzona konfiguracja na stronie Ustawienia

W sekcji **"Konfiguracja Modbus - Zaawansowane"** znajdziesz:

- **Typ rejestru**: Wybór między Holding Registers (FC03) a Input Registers (FC04)
- **Adres startowy rejestru**: Możliwość ustawienia od którego rejestru zacząć odczyt (0-10)
- **Liczba rejestrów do odczytu**: Ile rejestrów odczytać (1-10)
- **Mapowanie danych**: Jak interpretować odczytane dane:
  - HT73: Rejestr 0=Wilgotność, Rejestr 1=Temperatura
  - HT73v2: Rejestr 1=Temperatura, Rejestr 2=Wilgotność
  - PLC: Rejestr 0=Siłownik, Rejestr 4=TempEXT, Rejestr 5=TempCO
  - Niestandardowe: Własna interpretacja
- **Dzielnik wartości**: Jak podzielić odczytane wartości:
  - Brak (230 → 230)
  - ÷10 (230 → 23.0)
  - ÷100 (2300 → 23.0)

### 2. Narzędzia diagnostyczne

Na stronie **Diagnostyka** znajdziesz nowe narzędzia:

- **🔍 Skanuj Slave ID (1-10)**: Automatycznie znajdź wszystkie urządzenia Modbus w sieci
- **📖 Test FC03**: Testuj odczyt Holding Registers
- **📖 Test FC04**: Testuj odczyt Input Registers
- **📊 Informacje diagnostyczne**: Wyświetla aktualną konfigurację Modbus
- **📋 Tabela 10 rejestrów**: Rozszerzona z 6 do 10 rejestrów z różnymi formatami wartości

### 3. Nowe endpointy API

- `/api/modbus-scan` - Skanowanie urządzeń Slave ID 1-10
- `/api/modbus-test?fc=3` - Test FC03
- `/api/modbus-test?fc=4` - Test FC04
- `/api/settings` - Zwraca/zapisuje zaawansowane parametry

## 🔧 Jak używać

### Krok 1: Znajdź swoje urządzenie

1. Wejdź na stronę **Diagnostyka**
2. Kliknij **"🔍 Skanuj Slave ID (1-10)"**
3. Poczekaj na wyniki skanowania (może zająć do 10 sekund)
4. Zanotuj:
   - Slave ID urządzenia
   - Czy odpowiada na FC03 czy FC04

**UWAGA**: Skanowanie tymczasowo zmienia konfigurację Modbus, więc połączenie z aktualnym urządzeniem może zostać przerwane na czas skanowania.

### Krok 2: Skonfiguruj parametry

1. Wejdź na stronę **Ustawienia**
2. W sekcji **"Konfiguracja Modbus"** ustaw:
   - **Unit ID**: Slave ID znalezione w kroku 1
   - **Baudrate**: Sprawdź w dokumentacji urządzenia (zazwyczaj 9600)

3. W sekcji **"Konfiguracja Modbus - Zaawansowane"** ustaw:
   - **Typ rejestru**: FC03 lub FC04 (według wyników skanowania)
   - **Adres startowy**: Zazwyczaj 0
   - **Liczba rejestrów**: Zazwyczaj 2 dla HT73
   - **Mapowanie danych**: Wybierz odpowiednie dla twojego urządzenia
   - **Dzielnik wartości**: Zazwyczaj ÷10

4. Kliknij **"💾 Zapisz ustawienia"**
5. ESP32 zrestartuje się

### Krok 3: Weryfikacja

1. Wejdź na stronę **Dashboard**
2. Sprawdź czy dane się wyświetlają
3. Jeśli nie, wróć do **Diagnostyka** i sprawdź:
   - Czy status pokazuje "Połączony"
   - Czy wartości w tabeli rejestrów mają sens
   - Czy "Informacje diagnostyczne" pokazują prawidłową konfigurację

## 🔍 Przykładowe konfiguracje

### Czujnik HT73 (wersja standardowa)
```
Typ rejestru: Holding Registers (FC03)
Adres startowy: 0
Liczba rejestrów: 2
Mapowanie: HT73 (Reg0=Wilgotność, Reg1=Temperatura)
Dzielnik: ÷10
```

### Czujnik HT73 (wersja alternatywna)
```
Typ rejestru: Input Registers (FC04)
Adres startowy: 0
Liczba rejestrów: 2
Mapowanie: HT73 (Reg0=Wilgotność, Reg1=Temperatura)
Dzielnik: ÷10
```

### Czujnik HT73v2
```
Typ rejestru: Holding Registers (FC03)
Adres startowy: 0
Liczba rejestrów: 3
Mapowanie: HT73v2 (Reg1=Temperatura, Reg2=Wilgotność)
Dzielnik: ÷10
```

### PLC (kontroler PLC)
```
Typ rejestru: Holding Registers (FC03)
Adres startowy: 0
Liczba rejestrów: 6
Mapowanie: PLC
Dzielnik: ÷10
```

## 🐛 Rozwiązywanie problemów

### Problem: Urządzenie nie odpowiada

1. Sprawdź połączenie fizyczne RS485 (A, B, GND)
2. Upewnij się że baudrate jest poprawny
3. Użyj funkcji skanowania aby znaleźć Slave ID
4. Spróbuj obu typów rejestrów (FC03 i FC04)

### Problem: Wartości są nieprawidłowe

1. Sprawdź czy dzielnik jest ustawiony poprawnie
2. Spróbuj różnych mapowań danych
3. Zobacz surowe wartości w tabeli rejestrów
4. Porównaj z dokumentacją urządzenia

### Problem: Dane nie odświeżają się

1. Sprawdź status połączenia na Dashboard
2. Zobacz logi w Diagnostyce
3. Spróbuj ręcznie testów FC03/FC04
4. Zrestartuj ESP32

## 📚 Dokumentacja techniczna

### Ważne informacje o mapowaniu rejestrów

**Indeksy względne**: System odczytuje rejestry zaczynając od `modbusStartRegister`. W tablicy `modbusRawRegisters`:
- `modbusRawRegisters[0]` zawiera wartość z rejestru `modbusStartRegister`
- `modbusRawRegisters[1]` zawiera wartość z rejestru `modbusStartRegister + 1`
- itd.

**Przykład**: Jeśli ustawisz `modbusStartRegister=5` i `modbusRegisterCount=2`:
- System odczyta rejestry 5 i 6 z urządzenia
- `modbusRawRegisters[0]` = rejestr 5
- `modbusRawRegisters[1]` = rejestr 6

**Dla mapowania PLC**: Mapowanie PLC zakłada że `modbusStartRegister=0`, ponieważ wymaga dostępu do konkretnych rejestrów (0, 4, 5). Jeśli używasz innej konfiguracji, wybierz mapowanie "Niestandardowe".

### Struktura Config (rozszerzona)

```cpp
struct Config {
  // ... istniejące pola ...
  
  // Nowe pola
  bool modbusUseInputRegisters;     // true=FC04, false=FC03
  uint16_t modbusStartRegister;     // 0-10
  uint16_t modbusRegisterCount;     // 1-10
  uint8_t modbusDataMapping;        // 0=HT73, 1=HT73v2, 2=PLC, 3=Custom
  uint8_t modbusDivider;            // 0=brak, 1=÷10, 2=÷100
};
```

### Mapowanie danych

- **0 (HT73)**: Reg0=Humidity, Reg1=Temperature
- **1 (HT73v2)**: Reg1=Temperature, Reg2=Humidity
- **2 (PLC)**: Reg0=Actuator, Reg4=TempEXT, Reg5=TempCO
- **3 (Custom)**: Reg0=TempEXT, Reg1=Humidity

## 📝 Changelog

### Wersja 1.1.0
- ✅ Dodano zaawansowaną konfigurację Modbus
- ✅ Dodano narzędzia diagnostyczne
- ✅ Rozszerzono tabelę rejestrów do 10
- ✅ Dodano funkcję skanowania Slave ID
- ✅ Dodano testy FC03/FC04

## 🤝 Wsparcie

W razie problemów:
1. Sprawdź ten dokument
2. Zobacz logi w Diagnostyce
3. Utwórz issue na GitHub z opisem problemu i logami
