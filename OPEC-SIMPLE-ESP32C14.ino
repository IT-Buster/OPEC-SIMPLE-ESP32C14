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
 * Hardware: EleTechSup ES32C14 (ESP32-C3)
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
    
    // Wczytaj krzywą grzewczą
    for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
      char key[16];
      snprintf(key, sizeof(key), "curve_%d_x", i);
      config.heatingCurve[i][0] = preferences.getFloat(key, DEFAULT_HEATING_CURVE[i][0]);
      
      snprintf(key, sizeof(key), "curve_%d_y", i);
      config.heatingCurve[i][1] = preferences.getFloat(key, DEFAULT_HEATING_CURVE[i][1]);
    }
    
    Serial.println("[Config] Wczytano konfigurację z Flash");
  } else {
    // Pierwsze uruchomienie - użyj domyślnych wartości
    Serial.println("[Config] Pierwsze uruchomienie - używam wartości domyślnych");
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
  
  // Zapisz krzywą grzewczą
  for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
    char key[16];
    snprintf(key, sizeof(key), "curve_%d_x", i);
    preferences.putFloat(key, config.heatingCurve[i][0]);
    
    snprintf(key, sizeof(key), "curve_%d_y", i);
    preferences.putFloat(key, config.heatingCurve[i][1]);
  }
  
  preferences.end();
  Serial.println("[Config] Konfiguracja zapisana do Flash");
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
  config.configInitialized = false;
  
  // Skopiuj domyślną krzywą grzewczą
  for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
    config.heatingCurve[i][0] = DEFAULT_HEATING_CURVE[i][0];
    config.heatingCurve[i][1] = DEFAULT_HEATING_CURVE[i][1];
  }
  
  saveConfig();
  Serial.println("[Config] Reset do wartości domyślnych");
}

// ===================================
// SETUP - Inicjalizacja systemu
// ===================================
void setup() {
  // Inicjalizacja portu szeregowego
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("  OPEC-SIMPLE-ESP32C14");
  Serial.println("  Sterownik systemu grzewczego");
  Serial.println("  Wersja: 1.0.0");
  Serial.println("========================================");
  Serial.println();
  
  // Informacje o chipie
  Serial.printf("Chip: %s\n", ESP.getChipModel());
  Serial.printf("Rdzenie: %d\n", ESP.getChipCores());
  Serial.printf("Częstotliwość: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("RAM: %d KB\n", ESP.getHeapSize() / 1024);
  Serial.println();
  
  // Wczytaj konfigurację
  loadConfig();
  
  // Konfiguracja WiFi
  setupWiFi();
  
  // Konfiguracja Modbus
  setupModbus();
  
  // Konfiguracja serwera WWW
  setupWebServer();
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("  System gotowy!");
  Serial.println("========================================");
  Serial.println();
  
  if (WiFi.getMode() == WIFI_AP) {
    Serial.printf("Połącz się z WiFi: %s\n", WiFi.softAPSSID().c_str());
    Serial.printf("Hasło: %s\n", AP_PASSWORD);
    Serial.printf("Otwórz: http://%s\n", WiFi.softAPIP().toString().c_str());
  } else {
    Serial.printf("Dostępny pod: http://%s.local\n", MDNS_HOSTNAME);
    Serial.printf("Lub: http://%s\n", WiFi.localIP().toString().c_str());
  }
  
  Serial.println();
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
