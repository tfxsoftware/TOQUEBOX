#include "ApiManager.h"
#include "HardwareManager.h"
#include "NetworkManager.h"
#include <SPIFFS.h>
#include <HTTPClient.h>

// Constructor
ApiManager::ApiManager() {
    // Nothing to initialize
}

// Helper function to format MAC address without colons
String ApiManager::formatMacWithoutColons(const String& macAddress) {
    String formattedMac = macAddress;
    formattedMac.replace(":", "");
    return formattedMac;
}

// Get MAC address based on connection type
String ApiManager::getMacAddress() {
    // Get MAC address based on connection type
    if (networkManager.isEthernetConnected()) {
        return ETH.macAddress();
    } else {
        return WiFi.macAddress();
    }
}

// Update relay mapping with data from API
bool ApiManager::updateRelayMapping(JsonArray& mappings) {
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
    bool result = hardwareManager.setRelayMapping(newMapping, mappingSize);
    
    // Free allocated memory
    delete[] newMapping;
    
    return result;
}

// Main function to get device mapping from API
void ApiManager::getDeviceMapping() {
    // Only proceed if we have a network connection
    if (!networkManager.isEthernetConnected() && WiFi.status() != WL_CONNECTED) {
        Serial.println("Cannot fetch device mapping: No network connection");
        Serial.println("Using default relay mapping as fallback");
        hardwareManager.setDefaultRelayMapping();
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
                        hardwareManager.setDefaultRelayMapping();
                    }
                } else {
                    Serial.println("Received empty mapping array, using default mapping");
                    hardwareManager.setDefaultRelayMapping();
                }
            } else {
                Serial.println("Response JSON does not contain hardwareModelPortMappings, using default mapping");
                hardwareManager.setDefaultRelayMapping();
            }
        } else {
            Serial.println("Failed to parse JSON response: " + String(error.c_str()));
            Serial.println("Using default relay mapping as fallback");
            hardwareManager.setDefaultRelayMapping();
        }
    } else {
        Serial.println("HTTP GET failed with error code: " + String(httpCode));
        Serial.println("Using default relay mapping as fallback");
        hardwareManager.setDefaultRelayMapping();
    }
    
    http.end();
}