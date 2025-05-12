#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "config.h"
#include "webpage.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(SERVER_PORT);

void setup() {
  Serial.begin(115200);
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    Serial.println("Starting AP mode...");
    setupAP(server);
    return;
  }

  String saved_ssid, saved_password;
  
  // Try to load credentials and connect
  if(loadWiFiCredentials(saved_ssid, saved_password)) {
    Serial.println("Found saved credentials");
    if(connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
      Serial.println("Connected to WiFi");
      return;
    } else {
      // Connection failed
      if(isFirstConnection()) {
        // If it's the first connection attempt and it failed, clear SPIFFS and restart
        clearSPIFFSAndRestart();
      } else {
        // If it's not the first connection, just restart
        Serial.println("Connection failed. Restarting...");
        ESP.restart();
      }
    }
  }
  
  // If we reach here, either no credentials or connection failed
  setupAP(server);
}

void loop() {
  Serial.println("im alive");
  delay(1000);
}

