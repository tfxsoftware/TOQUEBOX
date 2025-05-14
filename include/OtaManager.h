#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Update.h>

class OtaManager {
private:
    static constexpr const char* FIRMWARE_VERSION = "1.0.0";  
    static constexpr const char* GITHUB_API = "https://api.github.com/repos/tfxsoftware/TOQUEBOX/releases/latest";
    
    String githubToken;
    
    bool performUpdate(const String& url);
    
    static void updateProgressCb(size_t progress, size_t total);

public:
    OtaManager();
    
    void setGithubToken(const char* token);
    bool checkForUpdates();
    
    const char* getCurrentVersion() const;
};

extern OtaManager otaManager;

#endif 
