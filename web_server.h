#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

// ===================================
// Wersja firmware (dla API)
// ===================================
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.1.0"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif
#ifndef BUILD_TIME
#define BUILD_TIME __TIME__
#endif

// ===================================
// Funkcje serwera WWW
// ===================================
void setupWiFi();
void setupWebServer();

#endif // WEB_SERVER_H
