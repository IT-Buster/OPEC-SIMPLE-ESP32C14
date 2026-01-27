#include "thermostat.h"
#include "config.h"
#include "modbus_handler.h"

// ===================================
// Zmienne globalne
// ===================================
float targetTemp = 25.0;
uint16_t lastActuatorCmd = 0;

// ===================================
// Obliczanie temperatury zadanej z krzywej grzewczej
// Wykorzystuje interpolację liniową między punktami krzywej
// ===================================
float calculateTargetTemp(float tempExternal) {
  // Krzywa jest posortowana rosnąco po temperaturze zewnętrznej
  // Szukamy dwóch punktów między którymi znajduje się aktualna temperatura
  
  for (int i = 0; i < HEATING_CURVE_POINTS - 1; i++) {
    float x1 = config.heatingCurve[i][0];     // Temp. zewnętrzna punkt 1
    float y1 = config.heatingCurve[i][1];     // Temp. CO punkt 1
    float x2 = config.heatingCurve[i+1][0];   // Temp. zewnętrzna punkt 2
    float y2 = config.heatingCurve[i+1][1];   // Temp. CO punkt 2
    
    // Sprawdź czy temperatura mieści się między x1 a x2
    if (tempExternal >= x1 && tempExternal <= x2) {
      // Interpolacja liniowa: y = y1 + (y2-y1)/(x2-x1) * (x-x1)
      float slope = (y2 - y1) / (x2 - x1);
      float interpolated = y1 + slope * (tempExternal - x1);
      
      // Serial zajęty przez RS485 - brak logów
      
      return interpolated;
    }
  }
  
  // Temperatura poza zakresem krzywej - zwróć wartość skrajną
  if (tempExternal < config.heatingCurve[0][0]) {
    // Serial zajęty przez RS485 - brak logów
    return config.heatingCurve[0][1];
  } else {
    // Serial zajęty przez RS485 - brak logów
    return config.heatingCurve[HEATING_CURVE_POINTS-1][1];
  }
}

// ===================================
// Główna pętla termostatu
// Algorytm z histerezą:
//   - Gdy TempCO < (Nastawa - hysteresis) → otwórz siłownik (100%)
//   - Gdy TempCO > (Nastawa + hysteresis) → zamknij siłownik (0%)
//   - W zakresie histerezy → zachowaj ostatni stan
// ===================================
void runThermostat() {
  // Sprawdź czy termostat jest aktywny
  if (!config.thermostatActive) {
    return;
  }
  
  // Sprawdź czy mamy połączenie z Modbus
  if (!modbusConnected) {
    // Serial zajęty przez RS485 - brak logów
    return;
  }
  
  // Oblicz temperaturę zadaną z krzywej grzewczej
  targetTemp = calculateTargetTemp(tempEXT);
  
  // Oblicz różnicę między rzeczywistą a zadaną temperaturą
  float tempDiff = targetTemp - tempCO;
  
  // Serial zajęty przez RS485 - brak logów
  
  // Logika z histerezą
  if (tempCO < targetTemp - config.hysteresis) {
    // Za zimno - otwórz siłownik (100%)
    if (lastActuatorCmd != 100) {
      // Serial zajęty przez RS485 - brak logów
      writeActuatorPosition(100);
      lastActuatorCmd = 100;
    }
  } else if (tempCO > targetTemp + config.hysteresis) {
    // Za ciepło - zamknij siłownik (0%)
    if (lastActuatorCmd != 0) {
      // Serial zajęty przez RS485 - brak logów
      writeActuatorPosition(0);
      lastActuatorCmd = 0;
    }
  } else {
    // W zakresie histerezy - nie zmieniaj
    // Serial zajęty przez RS485 - brak logów
  }
}
