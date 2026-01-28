/*
 * OPEC-SIMPLE-ESP32C14
 * 
 * Sterownik systemu grzewczego dla ESP32-C3 z komunikacją Modbus RTU
 * 
 * Funkcje:
 * - Odczyt temperatury CO i zewnętrznej przez Modbus RTU
 * - Termostat z krzywą grzewczą i histerezą
 * - Interfejs webowy (Dashboard, konfiguracja krzywej, ustawienia)
 * - WiFi (tryb AP i STA)
 * - Zapis konfiguracji w pamięci Flash
 * 
 * Hardware: EletechSup ES32C14 (ESP32-C3)
 * 
 * Autor: IT-Buster
 * Licencja: MIT
 * Bazuje na projekcie: OPEC-SIMPLE (Node-RED)
 */

// Biblioteki systemowe ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>

// Biblioteki projektu
#include "config.h"
#include "modbus_handler.h"
#include "thermostat.h"
#include "web_server.h"

// ===================================
// Obiekt Preferences (zapis konfiguracji)
// ===================================
Preferences preferences;

// ===================================
// Globalna zmienna konfiguracji
// ===================================
Config config;

// ===================================
// Funkcja: Wczytanie konfiguracji z Flash
// ===================================
void loadConfig() {
  preferences.begin("opec-config", false);
  
  // Sprawdź czy konfiguracja jest zainicjalizowana
  config.configInitialized = preferences.getBool("initialized", false);
  
  if (config.configInitialized) {
    // Wczytaj zapisaną konfigurację
    preferences.getString("wifiSSID", config.wifiSSID, sizeof(config.wifiSSID));
    preferences.getString("wifiPassword", config.wifiPassword, sizeof(config.wifiPassword));
    config.modbusUnitID = preferences.getUChar("modbusUnitID", MODBUS_UNIT_ID);
    config.modbusBaudrate = preferences.getUInt("modbusBaudrate", MODBUS_BAUDRATE);
    config.hysteresis = preferences.getFloat("hysteresis", DEFAULT_HYSTERESIS);
    config.thermostatActive = preferences.getBool("thermostatActive", DEFAULT_THERMOSTAT_ACTIVE);
    config.modbusUseInputRegisters = preferences.getBool("modbusUseInputReg", false); // Domyślnie false (Holding Registers)
    
    // ===== Wczytaj nowe pola zaawansowane =====
    config.modbusStartRegister = preferences.getUShort("modbusStartReg", 0);
    config.modbusRegisterCount = preferences.getUShort("modbusRegCount", 2);
    config.modbusDataMapping = preferences.getUChar("modbusMapping", 0);
    config.modbusDivider = preferences.getUChar("modbusDivider", 1);
    
    // ===== Walidacja nowych pól =====
    if (config.modbusStartRegister > 10) config.modbusStartRegister = 0;
    if (config.modbusRegisterCount < 1 || config.modbusRegisterCount > 10) config.modbusRegisterCount = 2;
    if (config.modbusDataMapping > 3) config.modbusDataMapping = 0;
    if (config.modbusDivider > 2) config.modbusDivider = 1;
    
    // Wczytaj krzywą grzewczą
    for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
      char key[16];
      snprintf(key, sizeof(key), "curve_%d_x", i);
      config.heatingCurve[i][0] = preferences.getFloat(key, DEFAULT_HEATING_CURVE[i][0]);
      
      snprintf(key, sizeof(key), "curve_%d_y", i);
      config.heatingCurve[i][1] = preferences.getFloat(key, DEFAULT_HEATING_CURVE[i][1]);
    }
    
    // Serial zajęty przez RS485 - brak logów
  } else {
    // Pierwsze uruchomienie - użyj domyślnych wartości
    // Serial zajęty przez RS485 - brak logów
    resetConfig();
  }
  
  preferences.end();
}

// ===================================
// Funkcja: Zapis konfiguracji do Flash
// ===================================
void saveConfig() {
  preferences.begin("opec-config", false);
  
  preferences.putBool("initialized", true);
  preferences.putString("wifiSSID", config.wifiSSID);
  preferences.putString("wifiPassword", config.wifiPassword);
  preferences.putUChar("modbusUnitID", config.modbusUnitID);
  preferences.putUInt("modbusBaudrate", config.modbusBaudrate);
  preferences.putFloat("hysteresis", config.hysteresis);
  preferences.putBool("thermostatActive", config.thermostatActive);
  preferences.putBool("modbusUseInputReg", config.modbusUseInputRegisters);
  
  // ===== Zapisz nowe pola zaawansowane =====
  preferences.putUShort("modbusStartReg", config.modbusStartRegister);
  preferences.putUShort("modbusRegCount", config.modbusRegisterCount);
  preferences.putUChar("modbusMapping", config.modbusDataMapping);
  preferences.putUChar("modbusDivider", config.modbusDivider);
  
  // Zapisz krzywą grzewczą
  for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
    char key[16];
    snprintf(key, sizeof(key), "curve_%d_x", i);
    preferences.putFloat(key, config.heatingCurve[i][0]);
    
    snprintf(key, sizeof(key), "curve_%d_y", i);
    preferences.putFloat(key, config.heatingCurve[i][1]);
  }
  
  preferences.end();
  // Serial zajęty przez RS485 - brak logów
}

// ===================================
// Funkcja: Reset konfiguracji do wartości domyślnych
// ===================================
void resetConfig() {
  config.wifiSSID[0] = '\0';
  config.wifiPassword[0] = '\0';
  config.modbusUnitID = MODBUS_UNIT_ID;
  config.modbusBaudrate = MODBUS_BAUDRATE;
  config.hysteresis = DEFAULT_HYSTERESIS;
  config.thermostatActive = DEFAULT_THERMOSTAT_ACTIVE;
  
  // ===== NOWE - Domyślne wartości zaawansowane =====
  config.modbusUseInputRegisters = false;  // Domyślnie Holding Registers (FC03)
  config.modbusStartRegister = 0;          // Start od rejestru 0
  config.modbusRegisterCount = 2;          // Odczyt 2 rejestrów
  config.modbusDataMapping = 0;            // HT73: Reg0=Wilg, Reg1=Temp
  config.modbusDivider = 1;                // Dziel przez 10
  
  config.configInitialized = false;
  
  // Skopiuj domyślną krzywą grzewczą
  for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
    config.heatingCurve[i][0] = DEFAULT_HEATING_CURVE[i][0];
    config.heatingCurve[i][1] = DEFAULT_HEATING_CURVE[i][1];
  }
  
  saveConfig();
  // Serial zajęty przez RS485 - brak logów
}

// ===================================
// SETUP - Inicjalizacja systemu
// ===================================
void setup() {
  // ⚠️ UWAGA: Serial (UART0) używany przez RS485!
  // Serial Monitor NIE będzie działał - używaj WebUI do monitorowania
  // Inicjalizacja Serial zostanie wykonana w setupModbus()
  
  delay(1000);
  
  // Wczytaj konfigurację
  loadConfig();
  
  // Konfiguracja WiFi
  setupWiFi();
  
  // Konfiguracja Modbus (to zainicjalizuje Serial dla RS485)
  setupModbus();
  
  // Konfiguracja serwera WWW
  setupWebServer();
  
  // System gotowy - brak logów Serial (zajęty przez RS485)
  // Monitoruj przez WebUI: http://opec-esp32.local
}

// ===================================
// LOOP - Główna pętla programu
// ===================================
void loop() {
  static unsigned long lastModbusRead = 0;
  static unsigned long lastThermostatRun = 0;
  
  unsigned long currentMillis = millis();
  
  // Odczyt danych z Modbus co 15 sekund
  if (currentMillis - lastModbusRead >= MODBUS_READ_INTERVAL) {
    readModbusData();
    lastModbusRead = currentMillis;
  }
  
  // Uruchom termostat co 30 sekund (po odczycie Modbus)
  if (config.thermostatActive && 
      modbusConnected && 
      currentMillis - lastThermostatRun >= 30000) {
    runThermostat();
    lastThermostatRun = currentMillis;
  }
  
  // Krótki delay dla stabilności
  delay(100);
}
