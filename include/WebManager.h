#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "webpage.h"
#include "StorageManager.h"

class WebManager {
private:
    // Constants
    static constexpr int SERVER_PORT = 80;
    
    // Server instance
    AsyncWebServer server;
    
    // Server state
    bool isServerStarted;

public:
    // Constructor
    WebManager();
    
    // Server configuration
    void setupServer();
    
    // Route configuration for normal operation (not AP mode)
    void setupDefaultRoutes();
    
    // Start server if not already started
    void startServer();
    
    // Access methods
    AsyncWebServer& getServer();
    int getPort() const;
    bool isStarted() const;
};

// Global instance
extern WebManager webManager;

#endif // WEB_MANAGER_H 