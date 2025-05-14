#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

class MqttManager {
private:
    // MQTT Configuration Constants
    static constexpr const char* MQTT_HOST = "186.209.52.208";
    static constexpr int MQTT_PORT = 1883;
    
    // Buffer for MAC-based topic (12 chars + null terminator)
    char deviceTopic[13];
    
    // MQTT Client and connection objects
    WiFiClient espClient;
    PubSubClient mqttClient;
    
    // Reconnection parameters
    static constexpr long MQTT_RECONNECT_DELAY = 5000;  // 5 seconds between reconnection attempts
    unsigned long lastMqttReconnectAttempt = 0;
    
    // Private methods
    void generateDeviceTopic();
    bool reconnect();
    static void staticCallback(char* topic, byte* payload, unsigned int length);
    
    // Callback handler - actual implementation
    void messageCallback(char* topic, byte* payload, unsigned int length);
    
    // Static pointer to active instance for callback bridge
    static MqttManager* instance;

public:
    // Constructor
    MqttManager();
    
    // Public methods
    void setup();
    void handle();
    bool publish(const char* message);
    bool isConnected();
    
    // Accessor for the topic
    const char* getTopic() const;
};

// Global instance declaration
extern MqttManager mqttManager;

#endif // MQTT_MANAGER_H 