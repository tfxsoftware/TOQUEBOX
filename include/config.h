#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

// WiFi Configuration
extern const char* AP_SSID;
extern const char* WIFI_FILE;
extern const char* FIRST_CONNECT_FLAG;

// Server Configuration
extern const int SERVER_PORT;

// Connection Configuration
extern const int MAX_CONNECTION_ATTEMPTS;
extern const int CONNECTION_DELAY;  // milliseconds

// Function declarations
bool loadWiFiCredentials(String &ssid, String &password);
void saveWiFiCredentials(const char* ssid, const char* password);
void setupAP(AsyncWebServer &server);
bool connectToWiFi(const char* ssid, const char* password);
bool isFirstConnection();
void setFirstConnectionFlag(bool isFirst);
void clearSPIFFSAndRestart();

#endif // CONFIG_H 