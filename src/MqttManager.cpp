#include "MqttManager.h"
#include "HardwareManager.h"
#include "OtaManager.h"

MqttManager* MqttManager::instance = nullptr;

MqttManager mqttManager;

MqttManager::MqttManager() : mqttClient(espClient) {
    memset(deviceTopic, 0, sizeof(deviceTopic));
    
    instance = this;
}

void MqttManager::staticCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        instance->messageCallback(topic, payload, length);
    }
}

void MqttManager::messageCallback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.printf("Message received on topic %s: %s\n", topic, message);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.printf("JSON parsing failed: %s\n", error.c_str());
        return;
    }
    if (strcmp(topic, deviceTopic) == 0) {
    const char* data = doc["data"];
    if (!data) {
        Serial.println("No 'data' field in JSON message");
        return;
    }
    
    if (strncmp(data, "RELAY", 5) == 0) {
        int relayNum = atoi(data + 5) - 1;  
        
        if (relayNum >= 0 && relayNum < hardwareManager.getActiveRelayCount() && hardwareManager.getRelayPin(relayNum) > 0) {
            Serial.printf("Activating relay %d (GPIO %d)\n", relayNum + 1, hardwareManager.getRelayPin(relayNum));
            hardwareManager.pulseRelay(hardwareManager.getRelayPin(relayNum));
        } else {
                Serial.printf("Invalid relay number: %d\n", relayNum + 1);
            }
        }
    }
    else if (strcmp(topic, "commands") == 0) {
        const char* data = doc["data"];
        if (strcmp(data, "UPDATE") == 0) {
            Serial.println("Received update command");
            otaManager.checkForUpdates();
        }
    }
}

void MqttManager::generateDeviceTopic() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(deviceTopic, sizeof(deviceTopic), "%02X%02X%02X%02X%02X%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    Serial.printf("Generated MQTT topic from MAC: %s\n", deviceTopic);
}

void MqttManager::setup() {
    Serial.println("\nInitializing MQTT...");
    
    generateDeviceTopic();
    
    Serial.printf("Device Topic (MAC): %s\n", deviceTopic);
    Serial.printf("MQTT Server: %s:%d\n", MQTT_HOST, MQTT_PORT);
    
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(staticCallback);
    
    if (mqttClient.connect(deviceTopic)) {
        Serial.println("Connected to MQTT server");
        mqttClient.subscribe(deviceTopic);
        Serial.printf("Subscribed to topic: %s\n", deviceTopic);
        mqttClient.subscribe("commands");
        Serial.printf("Subscribed to topic: commands\n");
    } else {
        Serial.println("Failed to connect to MQTT server. Will retry in loop...");
    }
}

bool MqttManager::reconnect() {
    if (mqttClient.connect(deviceTopic)) {
        Serial.println("Reconnected to MQTT server");
        mqttClient.subscribe(deviceTopic);
        Serial.printf("Subscribed to topic: %s\n", deviceTopic);
        mqttClient.subscribe("commands");
        Serial.printf("Subscribed to topic: commands\n");
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
