#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ETH.h>
#include "web_manager.h"
#include "webpage.h"
#include "mqtt_manager.h"
#include <esp_ota_ops.h>
#include "network_manager.h"
#include "hardware_manager.h"
#include "storage_manager.h"
#include "api_manager.h"
// Define SERVER_PORT directly in case includes aren't working properly
#ifndef SERVER_PORT
#define SERVER_PORT 80
#endif

// Create AsyncWebServer object on port 80
AsyncWebServer server(SERVER_PORT);

// Global variable
extern bool isInAPMode;

void setup() {
  Serial.begin(115200);
  

  const esp_partition_t *running = esp_ota_get_running_partition();
  Serial.printf("[LOG] Current running partition: %s\n", running->label);
  
  // Initialize relays
  setupRelays();
  
  // Initialize reset button
  setupResetButton();
  
  // Initialize and check Ethernet first
  Serial.println("Initializing Ethernet...");
  setupEthernet();
  
  if (isEthernetConnected()) {
    Serial.println("Ethernet connected! No need for WiFi");
    // Try to get device mapping from API
    setupMQTT();
    return;
  }
  
  Serial.println("No Ethernet connection, trying WiFi...");
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    Serial.println("Starting AP mode...");
    setupAP(server);
    isInAPMode = true;
    return;
  }

  String saved_ssid, saved_password;
  
  // Try to load credentials and connect
  if(loadWiFiCredentials(saved_ssid, saved_password)) {
    Serial.println("Found saved credentials");
    if(connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
      Serial.println("Connected to WiFi");
      isInAPMode = false;
      // Try to get device mapping from API
      getDeviceMapping();
      setupMQTT();
      return;
    } else {
      // Connection failed
      if(isFirstConnection()) {
        // If it's the first connection attempt and it failed, clear SPIFFS and restart
        clearSPIFFSAndRestart();
      } else {
        // If it's not the first connection, start AP mode but keep trying to reconnect
        Serial.println("Connection failed. Starting AP mode while attempting to reconnect...");
        setupAP(server);
        isInAPMode = true;
      }
    }
  } else {
    // No credentials found, start AP mode
    Serial.println("No credentials found. Starting AP mode...");
    setupAP(server);
    isInAPMode = true;
  }
}

void loop() {
  // Always check reset button
  checkResetButton();

  if (!isInAPMode) {
    if (!isEthernetConnected()) {
      checkWiFiConnection(server);
    }
    // Handle MQTT if we have a connection (either WiFi or Ethernet)
    if (isEthernetConnected() || WiFi.status() == WL_CONNECTED) {
      handleMQTT();
    }
  }

  delay(100);
}
  
 



