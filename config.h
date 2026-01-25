#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===================================
// Piny RS485 dla ES32C14 (ESP32-C3)
// ===================================
#define RS485_RX_PIN    20        // UART RX
#define RS485_TX_PIN    21        // UART TX
#define RS485_DE_RE_PIN 19        // Direction Enable (DE) i Receive Enable (RE)
#define SERIAL_MODBUS   Serial1   // UART1

// ===================================
// Parametry Modbus
// ===================================
#define MODBUS_BAUDRATE      115200
#define MODBUS_UNIT_ID       2
#define MODBUS_TIMEOUT       1000  // ms
#define MODBUS_RETRY_COUNT   3
#define MODBUS_READ_INTERVAL 15000 // ms (15 sekund)

// ===================================
// Parametry WiFi AP (Access Point)
// ===================================
#define AP_SSID_PREFIX "OPEC-ESP32-"
#define AP_PASSWORD    "opec1234"
#define AP_IP          IPAddress(192, 168, 4, 1)
#define AP_GATEWAY     IPAddress(192, 168, 4, 1)
#define AP_SUBNET      IPAddress(255, 255, 255, 0)

// ===================================
// Parametry mDNS
// ===================================
#define MDNS_HOSTNAME  "opec-esp32"

// ===================================
// Parametry termostatu
// ===================================
#define DEFAULT_HYSTERESIS 1.0    // °C (histereza ±1°C)
#define DEFAULT_THERMOSTAT_ACTIVE true

// ===================================
// Domyślna krzywa grzewcza
// Temperatura zewnętrzna -> Temperatura CO
// ===================================
#define HEATING_CURVE_POINTS 9
const float DEFAULT_HEATING_CURVE[HEATING_CURVE_POINTS][2] = {
  {-20.0, 65.0},
  {-15.0, 60.0},
  {-10.0, 55.0},
  { -5.0, 50.0},
  {  0.0, 45.0},
  {  5.0, 40.0},
  { 10.0, 35.0},
  { 15.0, 30.0},
  { 20.0, 25.0}
};

// ===================================
// Struktura konfiguracji
// ===================================
struct Config {
  char wifiSSID[32];
  char wifiPassword[64];
  uint8_t modbusUnitID;
  uint32_t modbusBaudrate;
  float hysteresis;
  bool thermostatActive;
  float heatingCurve[HEATING_CURVE_POINTS][2]; // [temp zewn., temp CO]
  bool configInitialized; // Flaga pierwszego uruchomienia
};

// ===================================
// Globalna zmienna konfiguracji
// ===================================
extern Config config;

// ===================================
// Funkcje zarządzania konfiguracją
// ===================================
void loadConfig();
void saveConfig();
void resetConfig();

#endif // CONFIG_H
