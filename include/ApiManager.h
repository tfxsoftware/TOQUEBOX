#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ETH.h>

class ApiManager {
private:
    static constexpr const char* API_ENDPOINT_BASE = "https://toquetoque.nectarlabs.com.br/api";
    static constexpr const char* API_ENDPOINT_MAPPING = "hardware/{}/model-port-mapping";
    
    String formatMacWithoutColons(const String& macAddress);
    
    String getMacAddress();
    
    bool updateRelayMapping(JsonArray& mappings);

public:
    ApiManager();
    
    void getDeviceMapping();
};

extern ApiManager apiManager;

#endif 
