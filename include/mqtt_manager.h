#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

// MQTT Configuration
extern const char* MQTT_HOST;
extern const int MQTT_PORT;
extern const char* MQTT_TOPIC;

// Function declarations
void setupMQTT();
void handleMQTT();
bool publishMQTT(const char* message);
void onMQTTMessage(char* topic, byte* payload, unsigned int length);
bool isMQTTConnected();

#endif // MQTT_MANAGER_H 