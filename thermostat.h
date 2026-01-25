#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>

// ===================================
// Zmienne globalne termostatu
// ===================================
extern float targetTemp;        // Temperatura zadana (obliczona z krzywej)
extern uint16_t lastActuatorCmd; // Ostatnie polecenie do siłownika

// ===================================
// Funkcje termostatu
// ===================================
float calculateTargetTemp(float tempExternal);
void runThermostat();

#endif // THERMOSTAT_H
