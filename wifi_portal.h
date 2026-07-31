#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include "config.h"

extern const char PORTAL_HTML[] PROGMEM;

// Initialize WiFi: tries connecting to saved credentials, falls back to AP + Captive Portal
void wifiBegin();

// Checks if the system is currently running in Access Point configuration mode
bool isAPMode();

// Must be called in loop() to handle DNS requests during Captive Portal mode
void handleDNSTick();

#endif // WIFI_PORTAL_H
