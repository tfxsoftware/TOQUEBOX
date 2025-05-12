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