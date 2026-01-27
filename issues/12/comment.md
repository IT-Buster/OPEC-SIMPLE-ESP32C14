Witaj! 👋

Na podstawie zdjęcia i dokumentacji projektu, sprawdźmy czy podłączenie jest poprawne:

## ✅ Co wygląda dobrze:
- Widzę płytkę **EletechSup ES32C14** - to poprawny moduł z wbudowanym interfejsem RS485
- Moduł jest zamontowany i gotowy do użycia

## 🔍 Co powinienem sprawdzić na zdjęciu:

### 1️⃣ Zasilanie
Zgodnie z dokumentacją, ES32C14 wymaga:
- **12V/24V DC** na złączu P2 (+ i GND) LUB
- **AC 85-265V** na odpowiednim złączu

### 2️⃣ Połączenie RS485 → PLC Modbus
Najważniejsze połączenie! Powinno być w złączu **P13 (RS485)**:

| Zacisk P13 | → | PLC Modbus |
|------------|---|------------|
| **A** | → | A (+) |
| **B** | → | B (-) |
| **GND** | → | GND |

⚠️ **WAŻNE:** 
- ES32C14 ma **wbudowany konwerter RS485** - nie potrzebujesz dodatkowego modułu MAX485!
- GPIO 1, GPIO 3, GPIO 22 są wewnętrznie podłączone do wbudowanego transceivera RS485

### 3️⃣ Rezystor terminujący (opcjonalny)
Jeśli ESP32 jest **na końcu linii RS485**, dodaj rezystor 120Ω między A i B

### 4️⃣ DIP Switch
Sprawdź ustawienia **DIP switch** na płytce - musi być skonfigurowany dla interfejsu RS485 (szczegóły w [docs/ES32C14_HARDWARE.md](https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14/blob/main/docs/ES32C14_HARDWARE.md))

---

## 📖 Pomocne zasoby:
- [Szczegółowy schemat połączeń](https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14/blob/main/wiring_diagram.md)
- [Specyfikacja hardware ES32C14](https://github.com/IT-Buster/OPEC-SIMPLE-ESP32C14/blob/main/docs/ES32C14_HARDWARE.md)

**Czy możesz zrobić zdjęcie z bliższej odległości, żebym mógł sprawdzić:**
1. Podłączenie zasilania (złącze P2)?
2. Podłączenie RS485 (złącze P13 - terminale A, B, GND)?
3. Ustawienie DIP switch?

To pomoże mi dokładnie zweryfikować poprawność montażu! 🔧