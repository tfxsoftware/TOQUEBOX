#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ETH.h>
#include "config.h"
#include "webpage.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(SERVER_PORT);

void setup() {
  Serial.begin(115200);
  
  // Initialize reset button
  setupResetButton();
    // Initialize and check Ethernet first
  Serial.println("Initializing Ethernet...");
  setupEthernet();
  
  if (isEthernetConnected()) {
    Serial.println("Ethernet connected! No need for WiFi");
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

  // If Ethernet is connected, no need to manage WiFi
  if (isEthernetConnected()) {
    delay(100);
    return;
  }

  checkWiFiConnection(server);
  
  delay(100);  // Small delay to prevent overwhelming the CPU
}


