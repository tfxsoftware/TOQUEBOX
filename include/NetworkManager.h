#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <ESPAsyncWebServer.h>

class NetworkManager {
private:
    // WiFi Configuration
    static constexpr const char* AP_SSID = "TOQUEBOX-SETUP";
    static constexpr const char* WIFI_FILE = "/wifi_creds.txt";
    
    // Connection Configuration
    static constexpr int MAX_CONNECTION_ATTEMPTS = 20;
    static constexpr int CONNECTION_DELAY = 500;  // milliseconds
    
    // WiFi Reconnection Configuration
    static constexpr unsigned long RECONNECT_INTERVAL = 5000;  // Try to reconnect every 5 seconds
    static constexpr uint8_t MAX_RECONNECT_ATTEMPTS = 5;       // Maximum reconnection attempts before AP mode
    
    // Ethernet Pin Configuration
    static constexpr uint8_t ETH_ADDR = 1;     // PHY Address
    static constexpr int POWER_PIN = 17;       // GPIO17 - Power Pin
    static constexpr int MDC_PIN = 23;         // GPIO23 - MDC Pin
    static constexpr int MDIO_PIN = 18;        // GPIO18 - MDIO Pin
    
    // State variables
    uint8_t reconnectAttempts = 0;             // Current reconnection attempts counter
    bool ethConnected = false;                 // Ethernet connection status
    unsigned long lastReconnectAttempt = 0;    // Last reconnection attempt timestamp
    bool inAPMode = false;                     // Whether we're currently in AP mode
    
    // Private methods
    bool handleWiFiReconnection(AsyncWebServer &server);
    static void staticWiFiEventHandler(WiFiEvent_t event);
    void wifiEventHandler(WiFiEvent_t event);
    
    // Static instance pointer for the event handler
    static NetworkManager* instance;

public:
    // Constructor
    NetworkManager();
    
    // Network setup methods
    void setupEthernet();
    bool isEthernetConnected() const;
    bool connectToWiFi(const char* ssid, const char* password);
    void setupAP(AsyncWebServer &server);
    
    // Connection management
    void checkWiFiConnection(AsyncWebServer &server);
    
    // Getters and setters
    bool isInAPMode() const;
    void setAPMode(bool mode);
    
    // Get WiFi file path
    const char* getWiFiFilePath() const;
};

// Global instance
extern NetworkManager networkManager;

#endif // NETWORK_MANAGER_H 