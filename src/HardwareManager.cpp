#include "HardwareManager.h"
#include <SPIFFS.h>
#include <ESP.h>

// Create the global instance
HardwareManager hardwareManager;

// Constructor
HardwareManager::HardwareManager() {
    // Initialize with zeros (already done in header with initialization)
}

bool HardwareManager::setDefaultRelayMapping() {
    Serial.println("Setting default relay mapping");
    
    // Create a temporary array with the default mapping
    int defaultMapping[DEFAULT_RELAY_COUNT];
    for (int i = 0; i < DEFAULT_RELAY_COUNT; i++) {
        defaultMapping[i] = DEFAULT_RELAY_PINS[i];
    }
    
    // Call the regular mapping function
    return setRelayMapping(defaultMapping, DEFAULT_RELAY_COUNT);
}

bool HardwareManager::setRelayMapping(int* relayMapping, size_t mappingSize) {
    if (relayMapping == NULL || mappingSize == 0) {
        Serial.println("Error: Invalid mapping array");
        return false;
    }
    
    // Limit to maximum allowed relays
    if (mappingSize > MAX_RELAYS) {
        Serial.printf("Warning: Truncating mapping from %d to %d relays\n", mappingSize, MAX_RELAYS);
        mappingSize = MAX_RELAYS;
    }
    
    // First clean up existing pins (set to INPUT)
    for (int i = 0; i < MAX_RELAYS; i++) {
        if (relayPins[i] > 0) {
            digitalWrite(relayPins[i], LOW);
            pinMode(relayPins[i], INPUT);
        }
    }
    
    // Clear existing array first
    for (int i = 0; i < MAX_RELAYS; i++) {
        relayPins[i] = 0;
    }
    
    // Copy new mapping values
    for (size_t i = 0; i < mappingSize; i++) {
        relayPins[i] = relayMapping[i];
    }
    
    // Store the active count
    activeRelayCount = mappingSize;
    
    // Initialize pins with new mapping
    setupRelays();
    
    // Log the new mapping
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
    
    // Initialize all relay pins as outputs
    for (int i = 0; i < MAX_RELAYS; i++) {
        if (relayPins[i] > 0) {  // Check for valid GPIO pin
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
    // Check if this is a valid relay pin
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
    
    digitalWrite(relayPin, HIGH);  // Turn on
    delay(RELAY_PULSE_DURATION);   // Use constant instead of hardcoded value
    digitalWrite(relayPin, LOW);   // Turn off
    Serial.printf("Pulsed relay %d on GPIO%d\n", relayIndex + 1, relayPin);
}

bool HardwareManager::isRelayPulsing(int relayPin) {
    // Simple implementation since we're using blocking delay()
    return false;
}

void HardwareManager::checkResetButton() {
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

// Accessor methods
int HardwareManager::getRelayPin(size_t index) const {
    if (index < MAX_RELAYS) {
        return relayPins[index];
    }
    return -1; // Invalid index
}

size_t HardwareManager::getActiveRelayCount() const {
    return activeRelayCount;
} 