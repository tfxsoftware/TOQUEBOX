#include "network_manager.h"
#include "storage_manager.h"
#include "webpage.h"

// WiFi Configuration Constants
const char* AP_SSID = "TOQUEBOX-SETUP";
const char* WIFI_FILE = "/wifi_creds.txt";

// Connection Configuration Constants
const int MAX_CONNECTION_ATTEMPTS = 20;
const int CONNECTION_DELAY = 500;  // milliseconds

// WiFi Reconnection Configuration
const unsigned long RECONNECT_INTERVAL = 5000;  // Try to reconnect every 5 seconds
const uint8_t MAX_RECONNECT_ATTEMPTS = 5;       // Maximum reconnection attempts before AP mode
uint8_t reconnectAttempts = 0;                  // Current reconnection attempts counter
bool isInAPMode = false;                        // AP mode status
unsigned long lastReconnectAttempt = 0;         // Last reconnection attempt timestamp

// Ethernet Pin Configuration
const uint8_t ETH_ADDR = 1;     // PHY Address
const int POWER_PIN = 17;       // GPIO17 - Power Pin
const int MDC_PIN = 23;         // GPIO23 - MDC Pin
const int MDIO_PIN = 18;        // GPIO18 - MDIO Pin

// Ethernet connection flag
bool ethConnected = false;

void WiFiEvent(WiFiEvent_t event) {
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

void setupEthernet() {
  Serial.println("Setting up Ethernet...");
  Serial.println("Power Pin: GPIO" + String(POWER_PIN));
  Serial.println("MDC Pin: GPIO" + String(MDC_PIN));
  Serial.println("MDIO Pin: GPIO" + String(MDIO_PIN));
  
  WiFi.onEvent(WiFiEvent);  // Register Ethernet event handler
  
  ETH.begin(
    ETH_ADDR,           // PHY address
    POWER_PIN,          // Power pin
    MDC_PIN,            // MDC pin
    MDIO_PIN,          // MDIO pin
    ETH_PHY_LAN8720,   // PHY type
    ETH_CLOCK_GPIO0_IN // Clock mode
  );
  
  // Wait a bit for Ethernet to initialize
  delay(2000);
}

bool isEthernetConnected() {
  return ethConnected && ETH.linkSpeed() > 0;
}

bool connectToWiFi(const char* ssid, const char* password) {
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
    setFirstConnectionFlag(false);
  } else {
    Serial.println("Failed to connect to WiFi");
  }
  return connected;
}

void setupAP(AsyncWebServer &server) {
  WiFi.softAP(AP_SSID);  // Create an open access point
  Serial.println("Access Point Started (Open Network)");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

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
      
      saveWiFiCredentials(ssid.c_str(), password.c_str());
      request->send(200, "text/plain", "Credentials saved. ESP32 will restart...");
      delay(2000);
      ESP.restart();
    } else {
      Serial.println("Error: Invalid or missing parameters in form submission");
      request->send(400, "text/plain", "Invalid parameters");
    }
  });

  server.begin();
}

bool handleWiFiReconnection(AsyncWebServer &server) {
  String saved_ssid, saved_password;
  
  if (!loadWiFiCredentials(saved_ssid, saved_password)) {
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
    setupAP(server);
    reconnectAttempts = 0;  // Reset counter
    return false;
  }
  
  return false;
}

void checkWiFiConnection(AsyncWebServer &server) {
  if (WiFi.status() != WL_CONNECTED && !isInAPMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
      lastReconnectAttempt = currentMillis;
      Serial.println("\nWiFi connection lost. Attempting to reconnect...");
      if (handleWiFiReconnection(server)) {
        isInAPMode = false;
      } else {
        isInAPMode = true;
      }
    }
  }
} 