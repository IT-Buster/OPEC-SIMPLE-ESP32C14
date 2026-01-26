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
uint16_t actuatorPos = 0;

// ===================================
// Inicjalizacja Modbus RTU
// ===================================
void setupModbus() {
  // ES32C14 używa Serial (UART0) dla RS485
  // Zgodnie z dokumentacją producenta ElecTechSup
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
// Odczyt danych z PLC przez Modbus
// Rejestry:
//   0: Pozycja siłownika (uint16, 0-100)
//   4: Temperatura zewnętrzna (int16, /10)
//   5: Temperatura CO (int16, /10)
// ===================================
bool readModbusData() {
  uint8_t result;
  uint8_t retries = 0;
  
  // Retry logic - maksymalnie 3 próby
  while (retries < MODBUS_RETRY_COUNT) {
    result = modbus.readHoldingRegisters(0, 6); // Odczyt rejestrów 0-5
    
    if (result == modbus.ku8MBSuccess) {
      // Pomyślnie odczytano dane
      actuatorPos = modbus.getResponseBuffer(0);
      
      // Rejestry 4 i 5 zawierają temperatury jako int16 (dzielone przez 10)
      int16_t tempExtRaw = (int16_t)modbus.getResponseBuffer(4);
      int16_t tempCORaw = (int16_t)modbus.getResponseBuffer(5);
      
      tempEXT = tempExtRaw / 10.0;
      tempCO = tempCORaw / 10.0;
      
      modbusConnected = true;
      
      Serial.printf("[Modbus] Odczyt OK - TempCO: %.1f°C, TempEXT: %.1f°C, Siłownik: %d%%\n", 
                    tempCO, tempEXT, actuatorPos);
      
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
