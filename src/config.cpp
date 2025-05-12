#include "config.h"
#include "webpage.h"

// WiFi Configuration Constants
const char* AP_SSID = "TOQUEBOX-SETUP";
const char* WIFI_FILE = "/wifi_creds.txt";
const char* FIRST_CONNECT_FLAG = "/first_connect.flag";

// Server Configuration Constants
const int SERVER_PORT = 80;

// Connection Configuration Constants
const int MAX_CONNECTION_ATTEMPTS = 20;
const int CONNECTION_DELAY = 500;  // milliseconds

// WiFi Reconnection Configuration
const unsigned long RECONNECT_INTERVAL = 5000;  // Try to reconnect every 5 seconds
const uint8_t MAX_RECONNECT_ATTEMPTS = 5;       // Maximum reconnection attempts before AP mode
uint8_t reconnectAttempts = 0;                  // Current reconnection attempts counter
bool isInAPMode = false;                        // AP mode status
unsigned long lastReconnectAttempt = 0;         // Last reconnection attempt timestamp

// Hardware Configuration Constants
const int RESET_BUTTON_PIN = 13;  // Reset button on GPIO13

// Ethernet Pin Configuration
const uint8_t ETH_ADDR = 1;     // PHY Address
const int POWER_PIN = 17;       // GPIO17 - Power Pin
const int MDC_PIN = 23;         // GPIO23 - MDC Pin
const int MDIO_PIN = 18;        // GPIO18 - MDIO Pin

// RMII Interface (these are implicit in the ESP32 hardware configuration)
// GPIO0  - RMII_CLK
// GPIO25 - RMII_RXD0
// GPIO26 - RMII_RXD1
// GPIO27 - RMII_CRS_DV
// GPIO22 - RMII_TXD0
// GPIO21 - RMII_TXD1
// GPIO19 - RMII_TX_EN

// Button debounce variables
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 50ms debounce time
int lastButtonState = HIGH;

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

void setupResetButton() {
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Reset button initialized on pin " + String(RESET_BUTTON_PIN));
}

void checkResetButton() {
  int reading = digitalRead(RESET_BUTTON_PIN);

  // If the button state has changed, reset the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If enough time has passed since the last change
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button is pressed (LOW because of INPUT_PULLUP)
    if (reading == LOW) {
      Serial.println("\nReset button pressed!");
      Serial.println("Clearing SPIFFS and restarting...");
      SPIFFS.format();
      ESP.restart();
    }
  }

  lastButtonState = reading;
}

bool isFirstConnection() {
  return !SPIFFS.exists(FIRST_CONNECT_FLAG);
}

void setFirstConnectionFlag(bool isFirst) {
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

void clearSPIFFSAndRestart() {
  Serial.println("First connection failed. Clearing SPIFFS and restarting...");
  SPIFFS.format();
  ESP.restart();
}

bool loadWiFiCredentials(String &ssid, String &password) {
  if(SPIFFS.exists(WIFI_FILE)) {
    File file = SPIFFS.open(WIFI_FILE, "r");
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

void saveWiFiCredentials(const char* ssid, const char* password) {
  Serial.println("Saving WiFi credentials:");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  
  File file = SPIFFS.open(WIFI_FILE, "w");
  if(file) {
    file.println(ssid);
    file.println(password);
    file.close();
    Serial.println("Credentials saved successfully");
  } else {
    Serial.println("Failed to save credentials!");
  }
  // After successfully saving credentials, set the first connection flag
  setFirstConnectionFlag(true);
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
    
    // Check reset button during connection attempts
    checkResetButton();
  }
  Serial.println();
  
  bool connected = WiFi.status() == WL_CONNECTED;
  if (connected) {
    Serial.println("Successfully connected to WiFi!");
    // If connection successful, mark it as not first connection
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