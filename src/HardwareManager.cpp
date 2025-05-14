#include "HardwareManager.h"
#include <SPIFFS.h>
#include <ESP.h>

HardwareManager hardwareManager;

HardwareManager::HardwareManager() {
}

bool HardwareManager::setDefaultRelayMapping() {
    Serial.println("Setting default relay mapping");
    
    int defaultMapping[DEFAULT_RELAY_COUNT];
    for (int i = 0; i < DEFAULT_RELAY_COUNT; i++) {
        defaultMapping[i] = DEFAULT_RELAY_PINS[i];
    }
    
    return setRelayMapping(defaultMapping, DEFAULT_RELAY_COUNT);
}

bool HardwareManager::setRelayMapping(int* relayMapping, size_t mappingSize) {
    if (relayMapping == NULL || mappingSize == 0) {
        Serial.println("Error: Invalid mapping array");
        return false;
    }
    
    if (mappingSize > MAX_RELAYS) {
        Serial.printf("Warning: Truncating mapping from %d to %d relays\n", mappingSize, MAX_RELAYS);
        mappingSize = MAX_RELAYS;
    }
    
    for (int i = 0; i < MAX_RELAYS; i++) {
        if (relayPins[i] > 0) {
            digitalWrite(relayPins[i], LOW);
            pinMode(relayPins[i], INPUT);
        }
    }
    
    for (int i = 0; i < MAX_RELAYS; i++) {
        relayPins[i] = 0;
    }
    
    for (size_t i = 0; i < mappingSize; i++) {
        relayPins[i] = relayMapping[i];
    }
    
    activeRelayCount = mappingSize;
    
    setupRelays();
    
    Serial.printf("New relay mapping set with %d pins:\n", mappingSize);
    for (size_t i = 0; i < mappingSize; i++) {
        Serial.printf("  Relay %d: GPIO %d\n", i+1, relayPins[i]);
    }
    
    return true;
}

void HardwareManager::setupResetButton() {
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    Serial.println("Reset button initialized on pin " + String(RESET_BUTTON_PIN));
}

void HardwareManager::setupRelays() {
    Serial.println("\nInitializing Relays...");
    
    for (int i = 0; i < MAX_RELAYS; i++) {
        if (relayPins[i] > 0) {  
            pinMode(relayPins[i], OUTPUT);
            digitalWrite(relayPins[i], LOW);
        }
    }
     
    Serial.println("Relays initialized in LOW state:");
    for (int i = 0; i < MAX_RELAYS; i++) {
        if (relayPins[i] > 0) {
            Serial.println("- Relay " + String(i + 1) + ": GPIO" + String(relayPins[i]) + " [LOW]");
        }
    }
    
    Serial.println("Pulse duration set to: " + String(RELAY_PULSE_DURATION) + "ms");
}

void HardwareManager::pulseRelay(int relayPin) {
    bool validPin = false;
    int relayIndex = -1;
    
    for (int i = 0; i < MAX_RELAYS; i++) {
        if (relayPins[i] == relayPin) {
            validPin = true;
            relayIndex = i;
            break;
        }
    }
    
    if (!validPin) {
        Serial.printf("Error: Invalid relay pin %d\n", relayPin);
        return;
    }
    
    digitalWrite(relayPin, HIGH);  
    delay(RELAY_PULSE_DURATION);   
    digitalWrite(relayPin, LOW);   
    Serial.printf("Pulsed relay %d on GPIO%d\n", relayIndex + 1, relayPin);
}

bool HardwareManager::isRelayPulsing(int relayPin) {
    return false;
}

void HardwareManager::checkResetButton() {
    int reading = digitalRead(RESET_BUTTON_PIN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading == LOW) {
            Serial.println("\nReset button pressed!");
            Serial.println("Clearing SPIFFS and restarting...");
            SPIFFS.format();
            ESP.restart();
        }
    }

    lastButtonState = reading;
}

int HardwareManager::getRelayPin(size_t index) const {
    if (index < MAX_RELAYS) {
        return relayPins[index];
    }
    return -1; 
}

size_t HardwareManager::getActiveRelayCount() const {
    return activeRelayCount;
} 
