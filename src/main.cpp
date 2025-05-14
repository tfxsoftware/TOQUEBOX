#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ETH.h>
#include "WebManager.h"
#include "webpage.h"
#include "MqttManager.h"
#include <esp_ota_ops.h>
#include "NetworkManager.h"
#include "HardwareManager.h"
#include "StorageManager.h"
#include "ApiManager.h"
#include "OtaManager.h"

// Create API manager instance
ApiManager apiManager;

void setup() {
  Serial.begin(115200);
  
  const esp_partition_t *running = esp_ota_get_running_partition();
  Serial.printf("[LOG] Current running partition: %s\n", running->label);
  Serial.printf("[LOG] Current firmware version: %s\n", otaManager.getCurrentVersion());
  
  // Initialize WebManager (but don't start server yet)
  webManager.setupServer();
  
  // Initialize relays
  hardwareManager.setupRelays();
  
  // Initialize reset button
  hardwareManager.setupResetButton();
  
  // Initialize and check Ethernet first
  Serial.println("Initializing Ethernet...");
  networkManager.setupEthernet();
  
  if (networkManager.isEthernetConnected()) {
    Serial.println("Ethernet connected! No need for WiFi");
    // Try to get device mapping from API
    mqttManager.setup();
    
    // Check for updates if connected via Ethernet
    Serial.println("Checking for OTA updates...");
    otaManager.setGithubToken("your_github_token_here");
    otaManager.checkForUpdates();
    return;
  }
  
  Serial.println("No Ethernet connection, trying WiFi...");
  // Initialize SPIFFS
  if(!storageManager.begin()) {
    Serial.println("An error occurred while mounting SPIFFS");
    Serial.println("Starting AP mode...");
    networkManager.setupAP(webManager.getServer());
    return;
  }

  String saved_ssid, saved_password;
  
  // Try to load credentials and connect
  if(storageManager.loadWiFiCredentials(saved_ssid, saved_password)) {
    Serial.println("Found saved credentials");
    if(networkManager.connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
      Serial.println("Connected to WiFi");
      networkManager.setAPMode(false);
      // Try to get device mapping from API
      apiManager.getDeviceMapping();
      mqttManager.setup();
      
      // Check for updates if connected via WiFi
      Serial.println("Checking for OTA updates...");
      otaManager.setGithubToken("your_github_token_here");
      otaManager.checkForUpdates();
      return;
    } else {
      // Connection failed
      if(storageManager.isFirstConnection()) {
        // If it's the first connection attempt and it failed, clear SPIFFS and restart
        storageManager.clearSPIFFSAndRestart();
      } else {
        // If it's not the first connection, start AP mode but keep trying to reconnect
        Serial.println("Connection failed. Starting AP mode while attempting to reconnect...");
        networkManager.setupAP(webManager.getServer());
      }
    }
  } else {
    // No credentials found, start AP mode
    Serial.println("No credentials found. Starting AP mode...");
    networkManager.setupAP(webManager.getServer());
  }
}

void loop() {
  // Always check reset button
  hardwareManager.checkResetButton();

  if (!networkManager.isInAPMode()) {
    if (!networkManager.isEthernetConnected()) {
      networkManager.checkWiFiConnection(webManager.getServer());
    }
    // Handle MQTT if we have a connection (either WiFi or Ethernet)
    if (networkManager.isEthernetConnected() || WiFi.status() == WL_CONNECTED) {
      mqttManager.handle();
    }
  }

  delay(100);
}
  
 



