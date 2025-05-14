#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Update.h>

class OtaManager {
private:
    // Firmware version and GitHub API URL
    static constexpr const char* FIRMWARE_VERSION = "1.0.0";  // Change this to match your version
    static constexpr const char* GITHUB_API = "https://api.github.com/repos/tfxsoftware/TOQUEBOX/releases/latest";
    
    // Private member to store GitHub token
    String githubToken;
    
    // Private method for performing the actual update
    bool performUpdate(const String& url);
    
    // Update progress callback
    static void updateProgressCb(size_t progress, size_t total);

public:
    // Constructor
    OtaManager();
    
    // Public methods
    void setGithubToken(const char* token);
    bool checkForUpdates();
    
    // Get version method
    const char* getCurrentVersion() const;
};

// Global instance
extern OtaManager otaManager;

#endif // OTA_MANAGER_H 