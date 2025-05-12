#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>

extern const char* FIRST_CONNECT_FLAG;

// Function declarations
bool loadWiFiCredentials(String &ssid, String &password);
void saveWiFiCredentials(const char* ssid, const char* password);
bool isFirstConnection();
void setFirstConnectionFlag(bool isFirst);
void clearSPIFFSAndRestart();

#endif // STORAGE_MANAGER_H 