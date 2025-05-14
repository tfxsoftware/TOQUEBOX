#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "hardware_manager.h"

// API endpoints
extern const char* API_ENDPOINT_BASE;
extern const char* API_ENDPOINT_MAPPING;

// Function declarations
void getDeviceMapping();
bool updateRelayMapping(JsonArray& mappings);
String getMacAddress();

#endif // API_MANAGER_H
