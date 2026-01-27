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
  // ⚠️ KRYTYCZNE: ES32C14 używa Serial (UART0) dla RS485
  // Zgodnie z dokumentacją producenta EletechSup
  // Serial Monitor NIE będzie działał - używaj WebUI!
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
  
  // UWAGA: Serial zajęty przez RS485 - brak logów!
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
// Odczyt danych z czujnika HT73 przez Modbus
// Rejestry Holding Registers (0x03) dla HT73:
//   0: Wilgotność (int16, /10)
//   1: Temperatura (int16, /10)
// Rejestry Input Registers (0x04) dla starszej konfiguracji:
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
      // Tryb czujnika HT73 - Holding Registers 0-1 (nowa konfiguracja)
      result = modbus.readHoldingRegisters(0, 2); // Odczyt rejestrów 0-1 (wilgotność + temp)
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
        
        // Zapisz surowe wartości do tablicy
        modbusRawRegisters[0] = modbus.getResponseBuffer(0); // Rejestr 0: Wilgotność
        modbusRawRegisters[1] = modbus.getResponseBuffer(1); // Rejestr 1: Temperatura
        
        // Rejestr 0: Wilgotność × 10
        int16_t humRaw = (int16_t)modbusRawRegisters[0];
        float humidityValue = humRaw / 10.0;
        
        // Rejestr 1: Temperatura × 10
        int16_t tempRaw = (int16_t)modbusRawRegisters[1];
        float temperature = tempRaw / 10.0;
        
        // Dla kompatybilności z resztą systemu - zapisz temperaturę jako tempEXT
        tempEXT = temperature;
        humidity = humidityValue;
        
        // Wyzeruj pozostałe (dla kompatybilności)
        tempCO = 0.0;
        actuatorPos = 0;
        
        modbusConnected = true;
        
        // UWAGA: Brak logów Serial - Serial zajęty przez RS485!
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
      }
      
      return true;
    } else {
      // Błąd odczytu
      retries++;
      if (retries < MODBUS_RETRY_COUNT) {
        delay(MODBUS_RETRY_DELAY);
      }
    }
  }
  
  // Wszystkie próby nie powiodły się
  modbusConnected = false;
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
    // Serial zajęty przez RS485 - brak logów
    return true;
  } else {
    // Serial zajęty przez RS485 - brak logów
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
    // Serial zajęty przez RS485 - brak logów
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
      
      // Serial zajęty przez RS485 - brak logów
      
      break;
    } else {
      retries++;
      if (retries < MODBUS_RETRY_COUNT) {
        delay(MODBUS_RETRY_DELAY);
      }
    }
  }
  
  // Przywróć oryginalny Slave ID
  modbus.begin(originalSlaveID, Serial);
  
  if (!success) {
    modbusConnected = false;
    // Serial zajęty przez RS485 - brak logów
  }
  
  return success;
}
