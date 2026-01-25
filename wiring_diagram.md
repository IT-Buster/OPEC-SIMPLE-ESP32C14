# Schemat połączeń - OPEC-SIMPLE-ESP32C14

## Hardware
- **Mikrokontroler:** ElecTechSup ES32C14 (ESP32-C3)
- **Konwerter RS485:** Dowolny moduł MAX485 lub podobny
- **Zasilanie:** 5V przez USB lub zewnętrzne (stabilizowane)

## Połączenia RS485

### ESP32-C3 (ES32C14) → Moduł RS485
| ESP32 Pin | RS485 Pin | Opis                  |
|-----------|-----------|------------------------|
| GPIO 21   | DI (TX)   | Transmit Data         |
| GPIO 20   | RO (RX)   | Receive Data          |
| GPIO 19   | DE + RE   | Direction Enable      |
| GND       | GND       | Ground                |
| 5V        | VCC       | Zasilanie modułu      |

### Moduł RS485 → PLC Modbus
| RS485 Pin | PLC Pin | Kabel  |
|-----------|---------|--------|
| A         | A (+)   | Zielony|
| B         | B (-)   | Biały  |
| GND       | GND     | Czarny |

## Uwagi
1. **Rezystor terminujący:** Jeśli ESP32 jest na końcu linii Modbus, dodaj rezystor 120Ω między A i B
2. **Długość kabla:** Max 1200m dla RS485 przy 115200 baud (praktycznie ~100m)
3. **Skrętka:** Użyj skrętki ekranowanej dla odporności na zakłócenia
4. **Zasilanie:** Upewnij się, że GND jest wspólny dla wszystkich urządzeń
5. **Polaryzacja:** Jeśli nie działa, spróbuj zamienić A ↔ B

## Schemat ASCII
```
ESP32-C3 (ES32C14)          MAX485 Module              PLC Modbus
+------------------+        +--------------+         +-------------+
|                  |        |              |         |             |
| GPIO 21 (TX)  ---+------> | DI           |         |             |
| GPIO 20 (RX)  ---+<------ | RO           |         |             |
| GPIO 19 (DE)  ---+------> | DE + RE      |         |             |
|                  |        |              |         |             |
|                  |        | A  ----------+-------> | A (+)       |
|                  |        | B  ----------+-------> | B (-)       |
|                  |        |              |         |             |
| GND -----------+--+------> | GND ---------+-------> | GND         |
| 5V ------------+--+------> | VCC          |         |             |
+------------------+        +--------------+         +-------------+
                                                      [120Ω A-B]
```

## Testowanie połączenia
1. Podłącz zasilanie do ESP32 i modułu RS485
2. Sprawdź, czy diody LED na module RS485 migają przy transmisji
3. Użyj narzędzia Modbus Scanner do sprawdzenia komunikacji
4. Monitoruj Serial Monitor (115200 baud) w Arduino IDE dla logów

## Rozwiązywanie problemów
- **Brak komunikacji:** Sprawdź połączenia A/B, spróbuj zamienić
- **Błędy CRC:** Sprawdź terminację linii (rezystor 120Ω)
- **Sporadyczne błędy:** Dodaj kondensator 100nF między VCC i GND modułu RS485
- **Długie opóźnienia:** Skróć kabel lub zmniejsz baudrate
