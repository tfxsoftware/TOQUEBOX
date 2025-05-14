#include "NetworkManager.h"
#include "StorageManager.h"
#include "webpage.h"
#include "WebManager.h"
#include <ESP.h>

// Initialize static member
NetworkManager* NetworkManager::instance = nullptr;

// Create global instance
NetworkManager networkManager;

// Constructor
NetworkManager::NetworkManager() {
    // Set this instance as the active one for callbacks
    instance = this;
    
    // Initialize state variables
    reconnectAttempts = 0;
    ethConnected = false;
    lastReconnectAttempt = 0;
    inAPMode = false;
}

// Static callback bridge - redirects to instance method
void NetworkManager::staticWiFiEventHandler(WiFiEvent_t event) {
    // Make sure we have a valid instance
    if (instance) {
        instance->wifiEventHandler(event);
    }
}

void NetworkManager::wifiEventHandler(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("ETH Started");
            ETH.setHostname("TOQUEBOX");  // Set hostname for Ethernet
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("ETH Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.print("ETH MAC: ");
            Serial.print(ETH.macAddress());
            Serial.print(", IPv4: ");
            Serial.print(ETH.localIP());
            if (ETH.fullDuplex()) {
                Serial.print(", FULL_DUPLEX");
            }
            Serial.print(", ");
            Serial.print(ETH.linkSpeed());
            Serial.println("Mbps");
            ethConnected = true;
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("ETH Disconnected");
            ethConnected = false;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("ETH Stopped");
            ethConnected = false;
            break;
        default:
            break;
    }
}

void NetworkManager::setupEthernet() {
    Serial.println("Setting up Ethernet...");
    Serial.println("Power Pin: GPIO" + String(POWER_PIN));
    Serial.println("MDC Pin: GPIO" + String(MDC_PIN));
    Serial.println("MDIO Pin: GPIO" + String(MDIO_PIN));
    
    WiFi.onEvent(staticWiFiEventHandler);  // Register Ethernet event handler
    
    ETH.begin(
        ETH_ADDR,           // PHY address
        POWER_PIN,          // Power pin
        MDC_PIN,            // MDC pin
        MDIO_PIN,           // MDIO pin
        ETH_PHY_LAN8720,    // PHY type
        ETH_CLOCK_GPIO0_IN  // Clock mode
    );
    
    // Wait a bit for Ethernet to initialize
    delay(2000);
}

bool NetworkManager::isEthernetConnected() const {
    return ethConnected && ETH.linkSpeed() > 0;
}

bool NetworkManager::connectToWiFi(const char* ssid, const char* password) {
    Serial.print("Attempting to connect to WiFi network: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while(WiFi.status() != WL_CONNECTED && attempts < MAX_CONNECTION_ATTEMPTS) {
        delay(CONNECTION_DELAY);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected) {
        Serial.println("Successfully connected to WiFi!");
        storageManager.setFirstConnectionFlag(false);
    } else {
        Serial.println("Failed to connect to WiFi");
    }
    return connected;
}

void NetworkManager::setupAP(AsyncWebServer &server) {
    WiFi.softAP(AP_SSID);  // Create an open access point
    Serial.println("Access Point Started (Open Network)");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
    
    // Set AP mode flag
    inAPMode = true;

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });

    // Route to handle form submission
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
        String ssid, password;
        if(request->hasParam("ssid", true) && request->hasParam("password", true)) {
            ssid = request->getParam("ssid", true)->value();
            password = request->getParam("password", true)->value();
            
            Serial.println("\nReceived new WiFi credentials:");
            Serial.println("-------------------------");
            Serial.print("SSID: ");
            Serial.println(ssid);
            Serial.print("Password: ");
            Serial.println(password);
            Serial.println("-------------------------\n");
            
            storageManager.saveWiFiCredentials(ssid.c_str(), password.c_str());
            request->send(200, "text/plain", "Credentials saved. ESP32 will restart...");
            delay(2000);
            ESP.restart();
        } else {
            Serial.println("Error: Invalid or missing parameters in form submission");
            request->send(400, "text/plain", "Invalid parameters");
        }
    });

    // Start the server using WebManager to ensure proper state tracking
    webManager.startServer();
}

bool NetworkManager::handleWiFiReconnection(AsyncWebServer &server) {
    String saved_ssid, saved_password;
    
    if (!storageManager.loadWiFiCredentials(saved_ssid, saved_password)) {
        Serial.println("No WiFi credentials found!");
        return false;
    }

    Serial.printf("Reconnection attempt %d of %d\n", reconnectAttempts + 1, MAX_RECONNECT_ATTEMPTS);
    
    if (connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
        Serial.println("WiFi reconnected successfully!");
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        reconnectAttempts = 0;  // Reset counter on successful connection
        return true;
    }
    
    reconnectAttempts++;
    
    if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        Serial.println("Maximum reconnection attempts reached!");
        Serial.println("Switching to AP mode...");
        setupAP(server);  // This will start the server as needed
        reconnectAttempts = 0;  // Reset counter
        return false;
    }
    
    return false;
}

void NetworkManager::checkWiFiConnection(AsyncWebServer &server) {
    if (WiFi.status() != WL_CONNECTED && !inAPMode) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
            lastReconnectAttempt = currentMillis;
            Serial.println("\nWiFi connection lost. Attempting to reconnect...");
            if (handleWiFiReconnection(server)) {
                inAPMode = false;
            } else {
                inAPMode = true;
            }
        }
    }
}

bool NetworkManager::isInAPMode() const {
    return inAPMode;
}

void NetworkManager::setAPMode(bool mode) {
    inAPMode = mode;
}

const char* NetworkManager::getWiFiFilePath() const {
    return WIFI_FILE;
} 