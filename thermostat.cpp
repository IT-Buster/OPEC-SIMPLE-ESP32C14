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
      
      Serial.printf("[Termostat] Interpolacja: %.1f°C zewn. -> %.1f°C CO (między [%.1f,%.1f] i [%.1f,%.1f])\n",
                    tempExternal, interpolated, x1, y1, x2, y2);
      
      return interpolated;
    }
  }
  
  // Temperatura poza zakresem krzywej - zwróć wartość skrajną
  if (tempExternal < config.heatingCurve[0][0]) {
    Serial.printf("[Termostat] Temp. zewn. %.1f°C poniżej krzywej, zwracam %.1f°C CO\n",
                  tempExternal, config.heatingCurve[0][1]);
    return config.heatingCurve[0][1];
  } else {
    Serial.printf("[Termostat] Temp. zewn. %.1f°C powyżej krzywej, zwracam %.1f°C CO\n",
                  tempExternal, config.heatingCurve[HEATING_CURVE_POINTS-1][1]);
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
    Serial.println("[Termostat] Brak połączenia Modbus - pomijam");
    return;
  }
  
  // Oblicz temperaturę zadaną z krzywej grzewczej
  targetTemp = calculateTargetTemp(tempEXT);
  
  // Oblicz różnicę między rzeczywistą a zadaną temperaturą
  float tempDiff = targetTemp - tempCO;
  
  Serial.printf("[Termostat] TempCO: %.1f°C, Nastawa: %.1f°C, Różnica: %.1f°C, Histereza: ±%.1f°C\n",
                tempCO, targetTemp, tempDiff, config.hysteresis);
  
  // Logika z histerezą
  if (tempCO < targetTemp - config.hysteresis) {
    // Za zimno - otwórz siłownik (100%)
    if (lastActuatorCmd != 100) {
      Serial.println("[Termostat] Akcja: GRZEJ (siłownik 100%)");
      writeActuatorPosition(100);
      lastActuatorCmd = 100;
    }
  } else if (tempCO > targetTemp + config.hysteresis) {
    // Za ciepło - zamknij siłownik (0%)
    if (lastActuatorCmd != 0) {
      Serial.println("[Termostat] Akcja: STOP (siłownik 0%)");
      writeActuatorPosition(0);
      lastActuatorCmd = 0;
    }
  } else {
    // W zakresie histerezy - nie zmieniaj
    Serial.println("[Termostat] Akcja: UTRZYMUJ (w zakresie histerezy)");
  }
}
