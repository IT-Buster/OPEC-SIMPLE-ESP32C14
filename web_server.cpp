#include "web_server.h"
#include "config.h"
#include "modbus_handler.h"
#include "thermostat.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// ===================================
// Obiekt serwera WWW
// ===================================
AsyncWebServer server(80);

// ===================================
// Pomocnicza funkcja - generowanie unikalnego SSID
// ===================================
String getAPSSID() {
  uint64_t chipid = ESP.getEfuseMac();
  char ssid[32];
  snprintf(ssid, sizeof(ssid), "%s%04X", AP_SSID_PREFIX, (uint16_t)(chipid & 0xFFFF));
  return String(ssid);
}

// ===================================
// Konfiguracja WiFi (AP lub STA mode)
// ===================================
void setupWiFi() {
  // Sprawdź czy jest zapisana konfiguracja WiFi
  if (config.configInitialized && strlen(config.wifiSSID) > 0) {
    // Tryb STA - połącz z zapisaną siecią
    Serial.printf("[WiFi] Łączenie z siecią: %s\n", config.wifiSSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifiSSID, config.wifiPassword);
    
    // Czekaj max 20 sekund na połączenie
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.printf("[WiFi] Połączono! IP: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WiFi] RSSI: %d dBm\n", WiFi.RSSI());
      return;
    } else {
      Serial.println();
      Serial.println("[WiFi] Nie udało się połączyć, uruchamiam tryb AP");
    }
  }
  
  // Tryb AP - utwórz własny punkt dostępu
  String apSSID = getAPSSID();
  Serial.printf("[WiFi] Uruchamiam Access Point: %s\n", apSSID.c_str());
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  WiFi.softAP(apSSID.c_str(), AP_PASSWORD);
  
  Serial.printf("[WiFi] AP uruchomiony! IP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("[WiFi] Hasło: %s\n", AP_PASSWORD);
}

// ===================================
// Konfiguracja serwera WWW i endpointów API
// ===================================
void setupWebServer() {
  // Inicjalizacja SPIFFS (system plików dla WebUI)
  if (!SPIFFS.begin(true)) {
    Serial.println("[WebServer] Błąd montowania SPIFFS!");
    return;
  }
  Serial.println("[WebServer] SPIFFS zamontowany");
  
  // Serwowanie plików statycznych z SPIFFS
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  // ===================================
  // API Endpoint: GET /api/status
  // Zwraca aktualny stan systemu w JSON
  // ===================================
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;
    
    doc["tempCO"] = tempCO;
    doc["tempEXT"] = tempEXT;
    doc["tempTarget"] = targetTemp;
    doc["actuatorPos"] = actuatorPos;
    doc["thermostatActive"] = config.thermostatActive;
    doc["modbusConnected"] = modbusConnected;
    doc["wifiRSSI"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["chipModel"] = ESP.getChipModel();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // ===================================
  // API Endpoint: POST /api/curve
  // Aktualizacja krzywej grzewczej
  // ===================================
  server.on("/api/curve", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (error) {
        request->send(400, "text/plain", "Błąd parsowania JSON");
        return;
      }
      
      // Aktualizacja krzywej
      JsonArray curve = doc["curve"];
      if (curve.size() == HEATING_CURVE_POINTS) {
        for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
          config.heatingCurve[i][0] = curve[i][0];
          config.heatingCurve[i][1] = curve[i][1];
        }
        saveConfig();
        Serial.println("[WebServer] Krzywa grzewcza zaktualizowana");
        request->send(200, "text/plain", "OK");
      } else {
        request->send(400, "text/plain", "Nieprawidłowa liczba punktów krzywej");
      }
    }
  );
  
  // ===================================
  // API Endpoint: GET /api/curve
  // Pobieranie krzywej grzewczej
  // ===================================
  server.on("/api/curve", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<1024> doc;
    JsonArray curve = doc.createNestedArray("curve");
    
    for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
      JsonArray point = curve.createNestedArray();
      point.add(config.heatingCurve[i][0]);
      point.add(config.heatingCurve[i][1]);
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // ===================================
  // API Endpoint: POST /api/thermostat
  // Włączenie/wyłączenie termostatu
  // ===================================
  server.on("/api/thermostat", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (error) {
        request->send(400, "text/plain", "Błąd parsowania JSON");
        return;
      }
      
      if (doc.containsKey("active")) {
        config.thermostatActive = doc["active"];
        saveConfig();
        Serial.printf("[WebServer] Termostat %s\n", 
                      config.thermostatActive ? "WŁĄCZONY" : "WYŁĄCZONY");
        request->send(200, "text/plain", "OK");
      } else {
        request->send(400, "text/plain", "Brak parametru 'active'");
      }
    }
  );
  
  // ===================================
  // API Endpoint: POST /api/settings
  // Aktualizacja ustawień
  // ===================================
  server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (error) {
        request->send(400, "text/plain", "Błąd parsowania JSON");
        return;
      }
      
      // Aktualizacja WiFi
      if (doc.containsKey("wifiSSID")) {
        strlcpy(config.wifiSSID, doc["wifiSSID"], sizeof(config.wifiSSID));
      }
      if (doc.containsKey("wifiPassword")) {
        strlcpy(config.wifiPassword, doc["wifiPassword"], sizeof(config.wifiPassword));
      }
      
      // Aktualizacja Modbus
      if (doc.containsKey("modbusUnitID")) {
        config.modbusUnitID = doc["modbusUnitID"];
      }
      if (doc.containsKey("modbusBaudrate")) {
        config.modbusBaudrate = doc["modbusBaudrate"];
      }
      
      // Aktualizacja histerezy
      if (doc.containsKey("hysteresis")) {
        config.hysteresis = doc["hysteresis"];
      }
      
      saveConfig();
      Serial.println("[WebServer] Ustawienia zaktualizowane");
      request->send(200, "text/plain", "OK - restart wymagany dla WiFi/Modbus");
    }
  );
  
  // ===================================
  // API Endpoint: GET /api/settings
  // Pobieranie ustawień
  // ===================================
  server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;
    
    doc["wifiSSID"] = config.wifiSSID;
    // Nie wysyłamy hasła WiFi
    doc["modbusUnitID"] = config.modbusUnitID;
    doc["modbusBaudrate"] = config.modbusBaudrate;
    doc["hysteresis"] = config.hysteresis;
    doc["thermostatActive"] = config.thermostatActive;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // ===================================
  // API Endpoint: POST /api/reset
  // Reset ustawień do domyślnych
  // ===================================
  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("[WebServer] Reset konfiguracji do domyślnych");
    resetConfig();
    request->send(200, "text/plain", "Resetowanie... Urządzenie zostanie zrestartowane");
    delay(1000);
    ESP.restart();
  });
  
  // Uruchomienie serwera
  server.begin();
  Serial.println("[WebServer] Serwer HTTP uruchomiony na porcie 80");
  
  // Konfiguracja mDNS
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[mDNS] Dostępny pod: http://%s.local\n", MDNS_HOSTNAME);
  } else {
    Serial.println("[mDNS] Błąd inicjalizacji!");
  }
}
