#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===================================
// WERSJA FIRMWARE
// ===================================
#define FIRMWARE_VERSION "1.1.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// ============================================
// PINY RS485 DLA PŁYTKI ES32C14
// ============================================
// Zgodnie z przykładem producenta EletechSup:
// http://www.485io.com
// 
// ES32C14 ma WBUDOWANY moduł RS485 podłączony do Serial (UART0):
// - GPIO1 (TX) → RS485 TX (Data Input)
// - GPIO3 (RX) → RS485 RX (Receive Output) 
// - GPIO22    → RS485 DE/RE (Direction Enable)
//
// ⚠️ KRYTYCZNE: Serial (UART0) jest używany przez RS485!
// - Serial Monitor NIE będzie działał podczas komunikacji Modbus
// - TX/RX to domyślne piny Serial - nie definiujemy ich osobno
// - Używaj WebUI (http://opec-esp32.local) do monitorowania statusu
// - Debugging: Użyj interfejsu WWW lub LED diagnostycznego

// RS485 używa Serial (UART0) - domyślne piny TX(GPIO1)/RX(GPIO3)
// Tylko DE/RE wymaga definicji
#define RS485_DE_RE_PIN  22  // IO22 → DE+RE (Direction Enable)

// ============================================
// KONFIGURACJA MODBUS
// ============================================
#define MODBUS_BAUDRATE      9600    // HT73 domyślnie: 9600
#define MODBUS_UNIT_ID       5       // Domyślny Slave ID dla HT73
#define MODBUS_TIMEOUT       1000    // ms
#define MODBUS_RETRY_COUNT   3
#define MODBUS_READ_INTERVAL 15000   // ms (15 sekund)
#define MODBUS_RETRY_DELAY   500     // ms (opóźnienie między próbami)

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
  
  // ===== Konfiguracja zaawansowana Modbus =====
  bool modbusUseInputRegisters;     // true=FC04, false=FC03
  uint16_t modbusStartRegister;     // Adres startowy (0-10)
  uint16_t modbusRegisterCount;     // Liczba rejestrów (1-10)
  uint8_t modbusDataMapping;        // 0=HT73(Reg0=Wilg,Reg1=Temp), 1=HT73v2(Reg1=Temp,Reg2=Wilg), 2=PLC, 3=Custom
  uint8_t modbusDivider;            // 0=brak, 1=÷10, 2=÷100
  
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
