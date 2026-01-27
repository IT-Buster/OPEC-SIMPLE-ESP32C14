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
uint16_t modbusRawRegisters[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Zwiększono z 6 do 10

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
// Używa konfiguracji z config:
//   - modbusUseInputRegisters: true=FC04, false=FC03
//   - modbusStartRegister: adres startowy (0-10)
//   - modbusRegisterCount: liczba rejestrów (1-10)
//   - modbusDataMapping: sposób interpretacji danych
//   - modbusDivider: dzielnik wartości
// ===================================
bool readModbusData() {
  uint8_t result;
  uint8_t retries = 0;
  
  // Retry logic - maksymalnie 3 próby
  while (retries < MODBUS_RETRY_COUNT) {
    // Wybór typu odczytu na podstawie konfiguracji
    if (config.modbusUseInputRegisters) {
      result = modbus.readInputRegisters(config.modbusStartRegister, config.modbusRegisterCount);
    } else {
      result = modbus.readHoldingRegisters(config.modbusStartRegister, config.modbusRegisterCount);
    }
    
    if (result == modbus.ku8MBSuccess) {
      // Zapisz surowe wartości
      for (int i = 0; i < config.modbusRegisterCount && i < 10; i++) {
        modbusRawRegisters[i] = modbus.getResponseBuffer(i);
      }
      
      // Wyczyść pozostałe rejestry
      for (int i = config.modbusRegisterCount; i < 10; i++) {
        modbusRawRegisters[i] = 0;
      }
      
      // Przetwarzanie według mapowania
      processModbusData();
      
      modbusConnected = true;
      return true;
    }
    
    retries++;
    if (retries < MODBUS_RETRY_COUNT) {
      delay(MODBUS_RETRY_DELAY);
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

// ===================================
// Przetwarzanie danych według mapowania
// Używa konfiguracji:
//   - modbusDataMapping: 0=HT73, 1=HT73v2, 2=PLC, 3=Custom
//   - modbusDivider: 0=brak, 1=÷10, 2=÷100
// UWAGA: modbusRawRegisters[0] zawiera wartość z rejestru config.modbusStartRegister
// ===================================
void processModbusData() {
  float divider = 1.0;
  
  // Wybór dzielnika
  switch (config.modbusDivider) {
    case 0: divider = 1.0; break;    // Brak
    case 1: divider = 10.0; break;   // ÷10
    case 2: divider = 100.0; break;  // ÷100
    default: divider = 10.0; break;
  }
  
  // Mapowanie danych według konfiguracji
  // UWAGA: Indeksy są względne - modbusRawRegisters[0] = pierwszy odczytany rejestr
  switch (config.modbusDataMapping) {
    case 0: // HT73: Reg0=Wilgotność, Reg1=Temperatura (względem startRegister)
      if (config.modbusRegisterCount >= 2) {
        humidity = (int16_t)modbusRawRegisters[0] / divider;
        tempEXT = (int16_t)modbusRawRegisters[1] / divider;
      }
      tempCO = 0.0;
      actuatorPos = 0;
      break;
      
    case 1: // HT73v2: Reg1=Temperatura, Reg2=Wilgotność (względem startRegister)
      if (config.modbusRegisterCount >= 3) {
        tempEXT = (int16_t)modbusRawRegisters[1] / divider;
        humidity = (int16_t)modbusRawRegisters[2] / divider;
      }
      tempCO = 0.0;
      actuatorPos = 0;
      break;
      
    case 2: // PLC: Reg0=Siłownik, Reg4=TempEXT, Reg5=TempCO (względem startRegister)
      // Dla PLC zakładamy że startRegister=0 i odczytujemy min 6 rejestrów
      if (config.modbusRegisterCount >= 6 && config.modbusStartRegister == 0) {
        actuatorPos = modbusRawRegisters[0];
        tempEXT = (int16_t)modbusRawRegisters[4] / divider;
        tempCO = (int16_t)modbusRawRegisters[5] / divider;
      }
      humidity = 0.0;
      break;
      
    case 3: // Custom - użytkownik sam interpretuje na podstawie surowych danych
      // Domyślnie: Reg0=TempEXT, Reg1=Humidity (względem startRegister)
      if (config.modbusRegisterCount >= 1) {
        tempEXT = (int16_t)modbusRawRegisters[0] / divider;
      }
      if (config.modbusRegisterCount >= 2) {
        humidity = (int16_t)modbusRawRegisters[1] / divider;
      }
      tempCO = 0.0;
      actuatorPos = 0;
      break;
  }
}

// ===================================
// Funkcja testowa - skanuje Slave ID od 1 do 10
// Zwraca JSON z listą odnalezionych urządzeń
// ===================================
String scanModbusSlaves() {
  String result = "{\"scanned\":[";
  bool first = true;
  
  for (uint8_t slaveID = 1; slaveID <= 10; slaveID++) {
    modbus.begin(slaveID, Serial);
    
    // Test FC03
    uint8_t fc03_result = modbus.readHoldingRegisters(0, 1);
    // Test FC04
    uint8_t fc04_result = modbus.readInputRegisters(0, 1);
    
    if (fc03_result == modbus.ku8MBSuccess || fc04_result == modbus.ku8MBSuccess) {
      if (!first) result += ",";
      result += "{\"id\":";
      result += slaveID;
      result += ",\"fc03\":";
      result += (fc03_result == modbus.ku8MBSuccess) ? "true" : "false";
      result += ",\"fc04\":";
      result += (fc04_result == modbus.ku8MBSuccess) ? "true" : "false";
      result += "}";
      first = false;
    }
    
    delay(100);
  }
  
  result += "]}";
  
  // Przywróć oryginalny Slave ID
  modbus.begin(config.modbusUnitID, Serial);
  
  return result;
}
