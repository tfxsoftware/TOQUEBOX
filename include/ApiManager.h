#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ETH.h>

class ApiManager {
private:
    // API endpoint constants
    static constexpr const char* API_ENDPOINT_BASE = "https://xtg08wff-3333.brs.devtunnels.ms/";
    static constexpr const char* API_ENDPOINT_MAPPING = "api/hardware/{}/model-port-mapping";
    
    // Helper function to format MAC address without colons
    String formatMacWithoutColons(const String& macAddress);
    
    // Get MAC address based on connection type
    String getMacAddress();
    
    // Update relay mapping with data from API
    bool updateRelayMapping(JsonArray& mappings);

public:
    // Constructor
    ApiManager();
    
    // Public methods
    void getDeviceMapping();
};

#endif // API_MANAGER_H 