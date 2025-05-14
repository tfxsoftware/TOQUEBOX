#include "WebManager.h"
#include "StorageManager.h"

// Create global instance
WebManager webManager;

// Constructor
WebManager::WebManager() : server(SERVER_PORT), isServerStarted(false) {
    // Server is initialized with port in constructor initializer list
}

void WebManager::setupServer() {
    // Set up basic server with default routes
    setupDefaultRoutes();
    // Don't start server here - let NetworkManager handle it
}

void WebManager::setupDefaultRoutes() {
    // Set up any default routes for normal operation (not AP mode)
    // In this case, we don't have any specific routes for normal operation
    // as the AP mode routes are handled by NetworkManager
}

void WebManager::startServer() {
    if (!isServerStarted) {
        server.begin();
        isServerStarted = true;
        Serial.println("Web server started on port " + String(SERVER_PORT));
    }
}

AsyncWebServer& WebManager::getServer() {
    return server;
}

int WebManager::getPort() const {
    return SERVER_PORT;
}

bool WebManager::isStarted() const {
    return isServerStarted;
} 