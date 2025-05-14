#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "webpage.h"
#include "StorageManager.h"

class WebManager {
private:
    static constexpr int SERVER_PORT = 80;
    
    AsyncWebServer server;
    
    bool isServerStarted;

public:
    WebManager();
    
    void setupServer();
    
    void setupDefaultRoutes();
    
    void startServer();
    
    AsyncWebServer& getServer();
    int getPort() const;
    bool isStarted() const;
};

extern WebManager webManager;

#endif 
