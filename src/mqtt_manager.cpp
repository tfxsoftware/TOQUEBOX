#include "mqtt_manager.h"
#include "hardware_manager.h"
#include <ArduinoJson.h>

// MQTT Configuration Constants
const char* MQTT_HOST = "186.209.52.208";
const int MQTT_PORT = 1883;
char DEVICE_TOPIC[13];  // Buffer for MAC-based topic (12 chars + null terminator)

// External reference to the relay mapping array
extern int RELAY_PINS[MAX_RELAYS];

// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Reconnection parameters
const long MQTT_RECONNECT_DELAY = 5000;  // 5 seconds between reconnection attempts
unsigned long lastMQTTReconnectAttempt = 0;

// Generate topic from MAC address
void generateDeviceTopic() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  // Format as a continuous string of hex digits without colons
  snprintf(DEVICE_TOPIC, sizeof(DEVICE_TOPIC), "%02X%02X%02X%02X%02X%02X", 
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  Serial.printf("Generated MQTT topic from MAC: %s\n", DEVICE_TOPIC);
}

void onMQTTMessage(char* topic, byte* payload, unsigned int length) {
  // Create a null-terminated string from the payload
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.printf("Message received on topic %s: %s\n", topic, message);

  // Parse JSON message
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.printf("JSON parsing failed: %s\n", error.c_str());
    return;
  }

  // Get the data field
  const char* data = doc["data"];
  if (!data) {
    Serial.println("No 'data' field in JSON message");
    return;
  }
  
  // Check if it's a relay command (format: "RELAY1", "RELAY2", etc.)
  if (strncmp(data, "RELAY", 5) == 0) {
    // Extract the relay number
    int relayNum = atoi(data + 5) - 1;  // Convert to 0-based index
    
    // Check if it's a valid relay number
    if (relayNum >= 0 && relayNum < MAX_RELAYS && RELAY_PINS[relayNum] > 0) {
      Serial.printf("Activating relay %d (GPIO %d)\n", relayNum + 1, RELAY_PINS[relayNum]);
      pulseRelay(RELAY_PINS[relayNum]);
    } else {
      Serial.printf("Invalid relay number: %d\n", relayNum + 1);
    }
  }
}

void setupMQTT() {
  Serial.println("\nInitializing MQTT...");
  
  // Generate MAC-based topic
  generateDeviceTopic();
  
  Serial.printf("Device Topic (MAC): %s\n", DEVICE_TOPIC);
  Serial.printf("MQTT Server: %s:%d\n", MQTT_HOST, MQTT_PORT);
  
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(onMQTTMessage);
  
  // Initial connection attempt
  if (mqttClient.connect(DEVICE_TOPIC)) {
    Serial.println("Connected to MQTT server");
    mqttClient.subscribe(DEVICE_TOPIC);
    Serial.printf("Subscribed to topic: %s\n", DEVICE_TOPIC);
  } else {
    Serial.println("Failed to connect to MQTT server. Will retry in loop...");
  }
}

bool reconnectMQTT() {
  if (mqttClient.connect(DEVICE_TOPIC)) {
    Serial.println("Reconnected to MQTT server");
    mqttClient.subscribe(DEVICE_TOPIC);
    Serial.printf("Subscribed to topic: %s\n", DEVICE_TOPIC);
    return true;
  }
  return false;
}

void handleMQTT() {
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastMQTTReconnectAttempt > MQTT_RECONNECT_DELAY) {
      lastMQTTReconnectAttempt = now;
      Serial.println("Attempting MQTT reconnection...");
      if (reconnectMQTT()) {
        lastMQTTReconnectAttempt = 0;
      }
    }
  } else {
    mqttClient.loop();
  }
}

bool publishMQTT(const char* message) {
  if (!mqttClient.connected()) {
    return false;
  }
  return mqttClient.publish(DEVICE_TOPIC, message);
}

bool isMQTTConnected() {
  return mqttClient.connected();
} 