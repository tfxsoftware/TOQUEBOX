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

ApiManager apiManager;

void setup() {
  Serial.begin(115200);
  
  const esp_partition_t *running = esp_ota_get_running_partition();
  Serial.printf("[LOG] Current running partition: %s\n", running->label);
  Serial.printf("[LOG] Current firmware version: %s\n", otaManager.getCurrentVersion());
  
  webManager.setupServer();
  
  hardwareManager.setupRelays();
  
  hardwareManager.setupResetButton();
  
  Serial.println("Initializing Ethernet...");
  networkManager.setupEthernet();
  
  if (networkManager.isEthernetConnected()) {
    Serial.println("Ethernet connected! No need for WiFi");
    mqttManager.setup();
    
    return;
  }
  
  Serial.println("No Ethernet connection, trying WiFi...");
  if(!storageManager.begin()) {
    Serial.println("An error occurred while mounting SPIFFS");
    Serial.println("Starting AP mode...");
    networkManager.setupAP(webManager.getServer());
    return;
  }

  String saved_ssid, saved_password;
  
  if(storageManager.loadWiFiCredentials(saved_ssid, saved_password)) {
    Serial.println("Found saved credentials");
    if(networkManager.connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
      Serial.println("Connected to WiFi");
      networkManager.setAPMode(false);
      apiManager.getDeviceMapping();
      mqttManager.setup();
      
      return;
    } else {
      if(storageManager.isFirstConnection()) {
        storageManager.clearSPIFFSAndRestart();
      } else {
        Serial.println("Connection failed. Starting AP mode while attempting to reconnect...");
        networkManager.setupAP(webManager.getServer());
      }
    }
  } else {
    Serial.println("No credentials found. Starting AP mode...");
    networkManager.setupAP(webManager.getServer());
  }
}

void loop() {
  hardwareManager.checkResetButton();

  if (!networkManager.isInAPMode()) {
    if (!networkManager.isEthernetConnected()) {
      networkManager.checkWiFiConnection(webManager.getServer());
    }
    if (networkManager.isEthernetConnected() || WiFi.status() == WL_CONNECTED) {
      mqttManager.handle();
    }
  }

  delay(100);
}
  
 
