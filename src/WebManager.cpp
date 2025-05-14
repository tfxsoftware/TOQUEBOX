#include "WebManager.h"
#include "StorageManager.h"

WebManager webManager;

WebManager::WebManager() : server(SERVER_PORT), isServerStarted(false) {
}

void WebManager::setupServer() {
    setupDefaultRoutes();
}

void WebManager::setupDefaultRoutes() {
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
