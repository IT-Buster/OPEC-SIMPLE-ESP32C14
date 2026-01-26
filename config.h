#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// PINY RS485 DLA PŁYTKI ES32C14
// ============================================
// Zgodnie z przykładem producenta EleTechSup:
// http://www.485io.com
// 
// ES32C14 ma WBUDOWANY moduł RS485 podłączony do:
// - IO1 (GPIO1)  → RS485 TX (Data Input)
// - IO3 (GPIO3)  → RS485 RX (Receive Output) 
// - IO22 (GPIO22)→ RS485 DE/RE (Direction Enable)
//
// ⚠️ UWAGA dla ESP32-D0WD (klasyczny ESP32):
// GPIO 1 i 3 to UART0 (Serial), te same piny co USB Serial!
// Podczas komunikacji Modbus Serial Monitor nie będzie działał.
// Używaj WebUI do monitorowania statusu.

#define RS485_TX_PIN     1   // IO1 → TX (Serial/UART0)
#define RS485_RX_PIN     3   // IO3 → RX (Serial/UART0)
#define RS485_DE_RE_PIN  22  // IO22 → DE+RE (Direction Enable)

// ============================================
// KONFIGURACJA MODBUS
// ============================================
#define MODBUS_BAUDRATE      115200
#define MODBUS_UNIT_ID       2       // Domyślny Slave ID (musi być zgodny z konfiguracją PLC!)
#define MODBUS_TIMEOUT       1000    // ms
#define MODBUS_RETRY_COUNT   3
#define MODBUS_READ_INTERVAL 15000   // ms (15 sekund)

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
