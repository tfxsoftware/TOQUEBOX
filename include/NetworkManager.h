#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <ESPAsyncWebServer.h>

class NetworkManager {
private:
    static constexpr const char* AP_SSID = "TOQUEBOX-SETUP";
    static constexpr const char* WIFI_FILE = "/wifi_creds.txt";
    
    static constexpr int MAX_CONNECTION_ATTEMPTS = 20;
    static constexpr int CONNECTION_DELAY = 500;  
    
    static constexpr unsigned long RECONNECT_INTERVAL = 5000;  
    static constexpr uint8_t MAX_RECONNECT_ATTEMPTS = 5;       
    
    static constexpr uint8_t ETH_ADDR = 1;     
    static constexpr int POWER_PIN = 17;       
    static constexpr int MDC_PIN = 23;         
    static constexpr int MDIO_PIN = 18;        
    
    uint8_t reconnectAttempts = 0;             
    bool ethConnected = false;                 
    unsigned long lastReconnectAttempt = 0;    
    bool inAPMode = false;                     
    
    bool handleWiFiReconnection(AsyncWebServer &server);
    static void staticWiFiEventHandler(WiFiEvent_t event);
    void wifiEventHandler(WiFiEvent_t event);
    
    static NetworkManager* instance;

public:
    NetworkManager();
    
    void setupEthernet();
    bool isEthernetConnected() const;
    bool connectToWiFi(const char* ssid, const char* password);
    void setupAP(AsyncWebServer &server);
    
    void checkWiFiConnection(AsyncWebServer &server);
    
    bool isInAPMode() const;
    void setAPMode(bool mode);
    
    const char* getWiFiFilePath() const;
};

extern NetworkManager networkManager;

#endif 
