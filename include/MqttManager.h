#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

class MqttManager {
private:
    static constexpr const char* MQTT_HOST = "186.209.52.208";
    static constexpr int MQTT_PORT = 1883;
    
    char deviceTopic[13];
    
    WiFiClient espClient;
    PubSubClient mqttClient;
    
    static constexpr long MQTT_RECONNECT_DELAY = 5000;  
    unsigned long lastMqttReconnectAttempt = 0;
    
    void generateDeviceTopic();
    bool reconnect();
    static void staticCallback(char* topic, byte* payload, unsigned int length);
    
    void messageCallback(char* topic, byte* payload, unsigned int length);
    
    static MqttManager* instance;

public:
    MqttManager();
    
    void setup();
    void handle();
    bool publish(const char* message);
    bool isConnected();
    
    const char* getTopic() const;
};

extern MqttManager mqttManager;

#endif 
