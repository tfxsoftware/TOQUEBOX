#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>

class StorageManager {
private:
    static constexpr const char* FIRST_CONNECT_FLAG = "/first_connect.flag";

public:
    StorageManager();
    
    bool loadWiFiCredentials(String &ssid, String &password);
    void saveWiFiCredentials(const char* ssid, const char* password);
    
    bool isFirstConnection();
    void setFirstConnectionFlag(bool isFirst);
    
    void clearSPIFFSAndRestart();
    
    bool begin();
};

extern StorageManager storageManager;

#endif 
