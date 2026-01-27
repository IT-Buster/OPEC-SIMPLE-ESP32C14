#ifndef MODBUS_HANDLER_H
#define MODBUS_HANDLER_H

#include <Arduino.h>

// ===================================
// Dane odczytane z PLC przez Modbus
// ===================================
extern float tempCO;          // Temperatura CO (°C)
extern float tempEXT;         // Temperatura zewnętrzna (°C) - z czujnika HT73
extern float humidity;        // Wilgotność (%) - z czujnika HT73
extern uint16_t actuatorPos;  // Pozycja siłownika (0-100)
extern bool modbusConnected;  // Status połączenia Modbus
extern uint16_t modbusRawRegisters[6];  // Surowe wartości rejestrów 0-5

// ===================================
// Funkcje obsługi Modbus
// ===================================
void setupModbus();
bool readModbusData();
bool readModbusRawData(uint8_t slaveID, uint16_t startRegister, uint16_t count);
bool writeActuatorPosition(uint16_t position);
void preTransmission();   // Callback przed transmisją
void postTransmission();  // Callback po transmisji

#endif // MODBUS_HANDLER_H
