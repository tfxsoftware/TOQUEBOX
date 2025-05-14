#include "api_manager.h"
#include "hardware_manager.h"
#include "network_manager.h"
#include "storage_manager.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <AsyncJson.h>
#include <ESP.h>
#include <WiFi.h>
#include <ETH.h>
#include <HTTPClient.h>

// API endpoint constants
const char* API_ENDPOINT_BASE = "https://xtg08wff-3333.brs.devtunnels.ms/";
const char* API_ENDPOINT_MAPPING = "api/hardware/{}/model-port-mapping";

// External reference to the relay mapping array in hardware_manager.cpp
extern int RELAY_PINS[MAX_RELAYS];

// Helper function to format MAC address without colons
String formatMacWithoutColons(const String& macAddress) {
  String formattedMac = macAddress;
  formattedMac.replace(":", "");
  return formattedMac;
}

void getDeviceMapping() {
  // Only proceed if we have a network connection
  if (!isEthernetConnected() && WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot fetch device mapping: No network connection");
    Serial.println("Using default relay mapping as fallback");
    setDefaultRelayMapping();
    return;
  }

  // Get MAC address of the device and format it without colons
  String macAddress = getMacAddress();
  String formattedMac = formatMacWithoutColons(macAddress);
  
  // Create the endpoint URL with MAC address
  String mappingEndpoint = String(API_ENDPOINT_MAPPING);
  mappingEndpoint.replace("{}", formattedMac);
  
  // Complete URL for API call
  String url = String(API_ENDPOINT_BASE) + mappingEndpoint;
  
  Serial.println("Fetching device mapping from: " + url);
  
  HTTPClient http;
  http.begin(url);
  
  // Make GET request
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Received mapping response: " + payload);
    
    // Parse JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      if (doc.containsKey("hardwareModelPortMappings")) {
        JsonArray mappings = doc["hardwareModelPortMappings"];
        
        if (mappings.size() > 0) {
          // Update the mapping using the dynamic array
          if (updateRelayMapping(mappings)) {
            Serial.println("Relay mapping updated successfully");
          } else {
            Serial.println("Failed to update relay mapping, using default mapping");
            setDefaultRelayMapping();
          }
        } else {
          Serial.println("Received empty mapping array, using default mapping");
          setDefaultRelayMapping();
        }
      } else {
        Serial.println("Response JSON does not contain hardwareModelPortMappings, using default mapping");
        setDefaultRelayMapping();
      }
    } else {
      Serial.println("Failed to parse JSON response: " + String(error.c_str()));
      Serial.println("Using default relay mapping as fallback");
      setDefaultRelayMapping();
    }
  } else {
    Serial.println("HTTP GET failed with error code: " + String(httpCode));
    Serial.println("Using default relay mapping as fallback");
    setDefaultRelayMapping();
  }
  
  http.end();
}

bool updateRelayMapping(JsonArray& mappings) {
  // Get array size
  size_t mappingSize = mappings.size();
  
  if (mappingSize == 0) {
    Serial.println("Error: Empty mapping received");
    return false;
  }
  
  // Create dynamic array for new mapping
  int* newMapping = new int[mappingSize];
  if (!newMapping) {
    Serial.println("Memory allocation failed for new mapping");
    return false;
  }
  
  // Copy values from JsonArray to int array
  for (size_t i = 0; i < mappingSize; i++) {
    newMapping[i] = mappings[i];
  }
  
  // Print new mapping for debugging
  Serial.printf("New relay mapping received with %d pins:\n", mappingSize);
  for (size_t i = 0; i < mappingSize; i++) {
    Serial.printf("  Relay %d: GPIO %d\n", i+1, newMapping[i]);
  }
  
  // Update hardware relay mapping
  bool result = setRelayMapping(newMapping, mappingSize);
  
  // Free allocated memory
  delete[] newMapping;
  
  return result;
}

String getMacAddress() {
  // Get MAC address based on connection type
  if (isEthernetConnected()) {
    return ETH.macAddress();
  } else {
    return WiFi.macAddress();
  }
}


