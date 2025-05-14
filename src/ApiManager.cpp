#include "ApiManager.h"
#include "HardwareManager.h"
#include "NetworkManager.h"
#include <SPIFFS.h>
#include <HTTPClient.h>

ApiManager::ApiManager() {
}

String ApiManager::formatMacWithoutColons(const String& macAddress) {
    String formattedMac = macAddress;
    formattedMac.replace(":", "");
    return formattedMac;
}

String ApiManager::getMacAddress() {
    if (networkManager.isEthernetConnected()) {
        return ETH.macAddress();
    } else {
        return WiFi.macAddress();
    }
}

bool ApiManager::updateRelayMapping(JsonArray& mappings) {
    size_t mappingSize = mappings.size();
    
    if (mappingSize == 0) {
        Serial.println("Error: Empty mapping received");
        return false;
    }
    
    int* newMapping = new int[mappingSize];
    if (!newMapping) {
        Serial.println("Memory allocation failed for new mapping");
        return false;
    }
    
    for (size_t i = 0; i < mappingSize; i++) {
        newMapping[i] = mappings[i];
    }
    
    Serial.printf("New relay mapping received with %d pins:\n", mappingSize);
    for (size_t i = 0; i < mappingSize; i++) {
        Serial.printf("  Relay %d: GPIO %d\n", i+1, newMapping[i]);
    }
    
    bool result = hardwareManager.setRelayMapping(newMapping, mappingSize);
    
    delete[] newMapping;
    
    return result;
}

void ApiManager::getDeviceMapping() {
    if (!networkManager.isEthernetConnected() && WiFi.status() != WL_CONNECTED) {
        Serial.println("Cannot fetch device mapping: No network connection");
        Serial.println("Using default relay mapping as fallback");
        hardwareManager.setDefaultRelayMapping();
        return;
    }

    String macAddress = getMacAddress();
    String formattedMac = formatMacWithoutColons(macAddress);
    
    String mappingEndpoint = String(API_ENDPOINT_MAPPING);
    mappingEndpoint.replace("{}", formattedMac);
    
    String url = String(API_ENDPOINT_BASE) + mappingEndpoint;
    
    Serial.println("Fetching device mapping from: " + url);
    
    HTTPClient http;
    http.begin(url);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Received mapping response: " + payload);
        
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            if (doc.containsKey("hardwareModelPortMappings")) {
                JsonArray mappings = doc["hardwareModelPortMappings"];
                
                if (mappings.size() > 0) {
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
