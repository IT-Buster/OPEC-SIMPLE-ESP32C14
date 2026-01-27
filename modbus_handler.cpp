#include "modbus_handler.h"
#include "config.h"
#include <ModbusMaster.h>

// ===================================
// Obiekt Modbus
// ===================================
ModbusMaster modbus;

// ===================================
// Dane globalne
// ===================================
bool modbusConnected = false;
float tempCO = 0.0;
float tempEXT = 0.0;
float humidity = 0.0;
uint16_t actuatorPos = 0;
uint16_t modbusRawRegisters[6] = {0, 0, 0, 0, 0, 0};

// ===================================
// Inicjalizacja Modbus RTU
// ===================================
void setupModbus() {
  // ES32C14 używa Serial (UART0) dla RS485
  // Zgodnie z dokumentacją producenta EletechSup
  Serial.begin(config.modbusBaudrate, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  
  // Konfiguracja pinu DE/RE (GPIO 22)
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW); // Tryb RX (odbiór)
  
  Serial.println("[Modbus] Inicjalizacja zakończona");
  Serial.printf("[Modbus] Baudrate: %d, Unit ID: %d\n", 
                config.modbusBaudrate, config.modbusUnitID);
  Serial.printf("[Modbus] TX: GPIO%d, RX: GPIO%d, DE/RE: GPIO%d\n", 
                RS485_TX_PIN, RS485_RX_PIN, RS485_DE_RE_PIN);
  
  // Inicjalizacja biblioteki ModbusMaster
  modbus.begin(config.modbusUnitID, Serial);
  
  // Callback dla przełączania TX/RX
  modbus.preTransmission(preTransmission);
  modbus.postTransmission(postTransmission);
}

// ===================================
// Callback przed transmisją - przełącz na tryb TX
// ===================================
void preTransmission() {
  digitalWrite(RS485_DE_RE_PIN, HIGH); // DE/RE = HIGH → nadawanie
}

// ===================================
// Callback po transmisji - przełącz na tryb RX
// ===================================
void postTransmission() {
  digitalWrite(RS485_DE_RE_PIN, LOW);  // DE/RE = LOW → odbiór
}

// ===================================
// Odczyt danych z czujnika HT73 lub PLC przez Modbus
// Rejestry Input Registers (0x04) dla HT73:
//   2: Temperatura (int16, /10)
//   3: Wilgotność (int16, /10)
// Rejestry Holding Registers (0x03) dla PLC:
//   0: Pozycja siłownika (uint16, 0-100)
//   4: Temperatura zewnętrzna (int16, /10)
//   5: Temperatura CO (int16, /10)
// ===================================
bool readModbusData() {
  uint8_t result;
  uint8_t retries = 0;
  
  // Retry logic - maksymalnie 3 próby
  while (retries < MODBUS_RETRY_COUNT) {
    if (config.modbusUseInputRegisters) {
      // Tryb czujnika HT73/SHT35 - Input Registers
      result = modbus.readInputRegisters(2, 2); // Odczyt rejestrów 2-3 (temp + wilgotność)
    } else {
      // Tryb PLC - Holding Registers
      result = modbus.readHoldingRegisters(0, 6); // Odczyt rejestrów 0-5
    }
    
    if (result == modbus.ku8MBSuccess) {
      if (config.modbusUseInputRegisters) {
        // Pomyślnie odczytano dane z HT73
        // Wyczyść tablicę przed zapisem nowych danych
        for (int i = 0; i < 6; i++) {
          modbusRawRegisters[i] = 0;
        }
        
        // Zapisz surowe wartości do tablicy (przesunięcie indeksów!)
        modbusRawRegisters[2] = modbus.getResponseBuffer(0); // Rejestr 2 czujnika → indeks 0 bufora
        modbusRawRegisters[3] = modbus.getResponseBuffer(1); // Rejestr 3 czujnika → indeks 1 bufora
        
        // Rejestr 2: Temperatura × 10
        int16_t tempRaw = (int16_t)modbusRawRegisters[2];
        float temperature = tempRaw / 10.0;
        
        // Rejestr 3: Wilgotność × 10
        int16_t humRaw = (int16_t)modbusRawRegisters[3];
        float humidityValue = humRaw / 10.0;
        
        // Dla kompatybilności z resztą systemu - zapisz temperaturę jako tempEXT
        // tempEXT jest używany przez resztę systemu jako główna temperatura zewnętrzna
        tempEXT = temperature;
        humidity = humidityValue;
        
        modbusConnected = true;
        
        Serial.printf("[Modbus] Odczyt HT73 OK - Temp: %.1f°C, Wilgotność: %.1f%%\n", 
                      temperature, humidityValue);
        
        // DEBUG: Wyświetl surowe wartości
        Serial.println("[Modbus] === RAW DATA (HT73) ===");
        Serial.printf("  Reg[2]: 0x%04X (%5d) | int16: %6d | Temp: %6.1f°C\n", 
          modbusRawRegisters[2], 
          modbusRawRegisters[2],
          tempRaw,
          temperature
        );
        Serial.printf("  Reg[3]: 0x%04X (%5d) | int16: %6d | Humi: %6.1f%%\n", 
          modbusRawRegisters[3], 
          modbusRawRegisters[3],
          humRaw,
          humidityValue
        );
        Serial.println("[Modbus] =======================");
      } else {
        // Pomyślnie odczytano dane z PLC
        // Zapisz surowe wartości do tablicy
        for (int i = 0; i < 6; i++) {
          modbusRawRegisters[i] = modbus.getResponseBuffer(i);
        }
        
        actuatorPos = modbusRawRegisters[0];
        
        // Rejestry 4 i 5 zawierają temperatury jako int16 (dzielone przez 10)
        int16_t tempExtRaw = (int16_t)modbusRawRegisters[4];
        int16_t tempCORaw = (int16_t)modbusRawRegisters[5];
        
        tempEXT = tempExtRaw / 10.0;
        tempCO = tempCORaw / 10.0;
        
        modbusConnected = true;
        
        Serial.printf("[Modbus] Odczyt PLC OK - TempCO: %.1f°C, TempEXT: %.1f°C, Siłownik: %d%%\n", 
                      tempCO, tempEXT, actuatorPos);
        
        // DEBUG: Wyświetl surowe wartości
        Serial.println("[Modbus] === RAW DATA (PLC) ===");
        for (int i = 0; i < 6; i++) {
          Serial.printf("  Reg[%d]: 0x%04X (%5d) | int16: %6d | float: %6.1f\n", 
            i, 
            modbusRawRegisters[i], 
            modbusRawRegisters[i],
            (int16_t)modbusRawRegisters[i],
            ((int16_t)modbusRawRegisters[i]) / 10.0
          );
        }
        Serial.println("[Modbus] ================");
      }
      
      return true;
    } else {
      // Błąd odczytu
      retries++;
      Serial.printf("[Modbus] Błąd odczytu (próba %d/%d) - kod: 0x%02X\n", 
                    retries, MODBUS_RETRY_COUNT, result);
      delay(500); // Odczekaj przed kolejną próbą
    }
  }
  
  // Wszystkie próby nie powiodły się
  modbusConnected = false;
  Serial.println("[Modbus] Połączenie nieudane po wszystkich próbach!");
  return false;
}

// ===================================
// Zapis pozycji siłownika do PLC
// Rejestr 4: Sterowanie siłownikiem
// ===================================
bool writeActuatorPosition(uint16_t position) {
  // Ograniczenie wartości do zakresu 0-100
  if (position > 100) {
    position = 100;
  }
  
  uint8_t result = modbus.writeSingleRegister(4, position);
  
  if (result == modbus.ku8MBSuccess) {
    Serial.printf("[Modbus] Zapis siłownika: %d%%\n", position);
    return true;
  } else {
    Serial.printf("[Modbus] Błąd zapisu siłownika - kod: 0x%02X\n", result);
    return false;
  }
}

// ===================================
// Odczyt surowych danych z niestandardowym Slave ID
// Dla celów diagnostycznych
// ===================================
bool readModbusRawData(uint8_t slaveID, uint16_t startRegister, uint16_t count) {
  // Walidacja parametrów
  if (slaveID < 1 || slaveID > 247 || count < 1 || count > 125) {
    Serial.println("[Modbus] Nieprawidłowe parametry testu");
    return false;
  }
  
  // Tymczasowo zmień Slave ID
  uint8_t originalSlaveID = config.modbusUnitID;
  modbus.begin(slaveID, Serial);
  
  uint8_t result;
  uint8_t retries = 0;
  bool success = false;
  
  // Retry logic - maksymalnie 3 próby
  while (retries < MODBUS_RETRY_COUNT) {
    result = modbus.readHoldingRegisters(startRegister, count);
    
    if (result == modbus.ku8MBSuccess) {
      // Pomyślnie odczytano dane - zapisz do tablicy
      for (int i = 0; i < count && i < 6; i++) {
        modbusRawRegisters[i] = modbus.getResponseBuffer(i);
      }
      
      // Wyczyść pozostałe rejestry jeśli count < 6
      for (int i = count; i < 6; i++) {
        modbusRawRegisters[i] = 0;
      }
      
      modbusConnected = true;
      success = true;
      
      Serial.printf("[Modbus] Test odczytu OK - SlaveID: %d, Start: %d, Count: %d\n", 
                    slaveID, startRegister, count);
      
      // Szczegółowe logowanie
      Serial.println("[Modbus] === RAW DATA (TEST) ===");
      for (int i = 0; i < count && i < 6; i++) {
        Serial.printf("  Reg[%d]: 0x%04X (%d) | int16: %d\n", 
          startRegister + i, 
          modbusRawRegisters[i], 
          modbusRawRegisters[i],
          (int16_t)modbusRawRegisters[i]
        );
      }
      Serial.println("[Modbus] ====================");
      
      break;
    } else {
      retries++;
      Serial.printf("[Modbus] Błąd testu (próba %d/%d) - kod: 0x%02X\n", 
                    retries, MODBUS_RETRY_COUNT, result);
      delay(500);
    }
  }
  
  // Przywróć oryginalny Slave ID
  modbus.begin(originalSlaveID, Serial);
  
  if (!success) {
    modbusConnected = false;
    Serial.println("[Modbus] Test zakończony niepowodzeniem!");
  } else {
    Serial.printf("[Modbus] Test zakończony sukcesem, przywrócono Slave ID: %d\n", originalSlaveID);
  }
  
  return success;
}
