#include "StorageManager.h"
#include "NetworkManager.h"

StorageManager storageManager;

StorageManager::StorageManager() {
}

bool StorageManager::begin() {
    return SPIFFS.begin(true);
}

bool StorageManager::loadWiFiCredentials(String &ssid, String &password) {
    if(SPIFFS.exists(networkManager.getWiFiFilePath())) {
        File file = SPIFFS.open(networkManager.getWiFiFilePath(), "r");
        if(file) {
            ssid = file.readStringUntil('\n');
            password = file.readStringUntil('\n');
            file.close();
            ssid.trim();
            password.trim();
            return true;
        }
    }
    return false;
}

void StorageManager::saveWiFiCredentials(const char* ssid, const char* password) {
    Serial.println("Saving WiFi credentials:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);
    
    File file = SPIFFS.open(networkManager.getWiFiFilePath(), "w");
    if(file) {
        file.println(ssid);
        file.println(password);
        file.close();
        Serial.println("Credentials saved successfully");
    } else {
        Serial.println("Failed to save credentials!");
    }
    setFirstConnectionFlag(true);
}

bool StorageManager::isFirstConnection() {
    return !SPIFFS.exists(FIRST_CONNECT_FLAG);
}

void StorageManager::setFirstConnectionFlag(bool isFirst) {
    if (isFirst) {
        if (SPIFFS.exists(FIRST_CONNECT_FLAG)) {
            SPIFFS.remove(FIRST_CONNECT_FLAG);
        }
    } else {
        File flagFile = SPIFFS.open(FIRST_CONNECT_FLAG, "w");
        if (flagFile) {
            flagFile.println("1");
            flagFile.close();
        }
    }
}

void StorageManager::clearSPIFFSAndRestart() {
    Serial.println("First connection failed. Clearing SPIFFS and restarting...");
    SPIFFS.format();
    ESP.restart();
} 
