#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Update.h>

class OtaManager {
private:
    static constexpr const char* FIRMWARE_VERSION = "v1.0.0";  
    static constexpr const char* GITHUB_API = "https://api.github.com/repos/Nectar-Labs/toque_box/releases/latest";
    static constexpr const char* GITHUB_TOKEN = "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";
    
    bool performUpdate(const String& url);
    
    static void updateProgressCb(size_t progress, size_t total);

public:
    OtaManager();
    
    bool checkForUpdates();
    const char* getCurrentVersion() const;
};

extern OtaManager otaManager;

#endif 
