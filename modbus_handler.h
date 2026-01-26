#ifndef MODBUS_HANDLER_H
#define MODBUS_HANDLER_H

#include <Arduino.h>

// ===================================
// Dane odczytane z PLC przez Modbus
// ===================================
extern float tempCO;          // Temperatura CO (°C)
extern float tempEXT;         // Temperatura zewnętrzna (°C)
extern uint16_t actuatorPos;  // Pozycja siłownika (0-100)
extern bool modbusConnected;  // Status połączenia Modbus

// ===================================
// Funkcje obsługi Modbus
// ===================================
void setupModbus();
bool readModbusData();
bool writeActuatorPosition(uint16_t position);
void preTransmission();   // Callback przed transmisją
void postTransmission();  // Callback po transmisji

#endif // MODBUS_HANDLER_H
