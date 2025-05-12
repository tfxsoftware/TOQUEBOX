#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <ESPAsyncWebServer.h>

// WiFi Configuration
extern const char* AP_SSID;
extern const char* WIFI_FILE;

// WiFi Reconnection Configuration
extern const unsigned long RECONNECT_INTERVAL;     // milliseconds between reconnection attempts
extern const uint8_t MAX_RECONNECT_ATTEMPTS;       // maximum number of reconnection attempts before AP mode
extern uint8_t reconnectAttempts;                  // current number of reconnection attempts
extern bool isInAPMode;                           // Flag to track AP mode status
extern unsigned long lastReconnectAttempt;        // Last time we tried to reconnect

// Ethernet Pin Configuration
extern const uint8_t ETH_ADDR;
extern const int POWER_PIN;
extern const int MDC_PIN;
extern const int MDIO_PIN;

// Connection Configuration
extern const int MAX_CONNECTION_ATTEMPTS;
extern const int CONNECTION_DELAY;  // milliseconds

// Function declarations
void setupAP(AsyncWebServer &server);
bool connectToWiFi(const char* ssid, const char* password);
void setupEthernet();
bool isEthernetConnected();
void WiFiEvent(WiFiEvent_t event);
bool handleWiFiReconnection(AsyncWebServer &server);
void checkWiFiConnection(AsyncWebServer &server);

#endif // NETWORK_MANAGER_H 