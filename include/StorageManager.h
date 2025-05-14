#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>

class StorageManager {
private:
    // File paths and constants
    static constexpr const char* FIRST_CONNECT_FLAG = "/first_connect.flag";

public:
    // Constructor
    StorageManager();
    
    // WiFi credentials management
    bool loadWiFiCredentials(String &ssid, String &password);
    void saveWiFiCredentials(const char* ssid, const char* password);
    
    // First connection flag management
    bool isFirstConnection();
    void setFirstConnectionFlag(bool isFirst);
    
    // Utility methods
    void clearSPIFFSAndRestart();
    
    // Initialization
    bool begin();
};

// Global instance
extern StorageManager storageManager;

#endif // STORAGE_MANAGER_H 