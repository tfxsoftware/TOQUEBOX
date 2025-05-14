#include "MqttManager.h"
#include "HardwareManager.h"

// Initialize static member
MqttManager* MqttManager::instance = nullptr;

// Create the global instance
MqttManager mqttManager;

// Constructor
MqttManager::MqttManager() : mqttClient(espClient) {
    // Initialize deviceTopic with zeros
    memset(deviceTopic, 0, sizeof(deviceTopic));
    
    // Set this instance as the active one for callbacks
    instance = this;
}

// Static callback bridge - redirects to instance method
void MqttManager::staticCallback(char* topic, byte* payload, unsigned int length) {
    // Make sure we have a valid instance
    if (instance) {
        instance->messageCallback(topic, payload, length);
    }
}

void MqttManager::messageCallback(char* topic, byte* payload, unsigned int length) {
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
        if (relayNum >= 0 && relayNum < hardwareManager.getActiveRelayCount() && hardwareManager.getRelayPin(relayNum) > 0) {
            Serial.printf("Activating relay %d (GPIO %d)\n", relayNum + 1, hardwareManager.getRelayPin(relayNum));
            hardwareManager.pulseRelay(hardwareManager.getRelayPin(relayNum));
        } else {
            Serial.printf("Invalid relay number: %d\n", relayNum + 1);
        }
    }
}

void MqttManager::generateDeviceTopic() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    // Format as a continuous string of hex digits without colons
    snprintf(deviceTopic, sizeof(deviceTopic), "%02X%02X%02X%02X%02X%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    Serial.printf("Generated MQTT topic from MAC: %s\n", deviceTopic);
}

void MqttManager::setup() {
    Serial.println("\nInitializing MQTT...");
    
    // Generate MAC-based topic
    generateDeviceTopic();
    
    Serial.printf("Device Topic (MAC): %s\n", deviceTopic);
    Serial.printf("MQTT Server: %s:%d\n", MQTT_HOST, MQTT_PORT);
    
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(staticCallback);
    
    // Initial connection attempt
    if (mqttClient.connect(deviceTopic)) {
        Serial.println("Connected to MQTT server");
        mqttClient.subscribe(deviceTopic);
        Serial.printf("Subscribed to topic: %s\n", deviceTopic);
    } else {
        Serial.println("Failed to connect to MQTT server. Will retry in loop...");
    }
}

bool MqttManager::reconnect() {
    if (mqttClient.connect(deviceTopic)) {
        Serial.println("Reconnected to MQTT server");
        mqttClient.subscribe(deviceTopic);
        Serial.printf("Subscribed to topic: %s\n", deviceTopic);
        return true;
    }
    return false;
}

void MqttManager::handle() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastMqttReconnectAttempt > MQTT_RECONNECT_DELAY) {
            lastMqttReconnectAttempt = now;
            Serial.println("Attempting MQTT reconnection...");
            if (reconnect()) {
                lastMqttReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient.loop();
    }
}

bool MqttManager::publish(const char* message) {
    if (!mqttClient.connected()) {
        return false;
    }
    return mqttClient.publish(deviceTopic, message);
}

bool MqttManager::isConnected() {
    return mqttClient.connected();
}

const char* MqttManager::getTopic() const {
    return deviceTopic;
} 