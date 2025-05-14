#include "hardware_manager.h"
#include "storage_manager.h"
#include <SPIFFS.h>

// Hardware Configuration Constants
const int RESET_BUTTON_PIN = 13;  // Reset button on GPIO13

// Initialize array with zeros
int RELAY_PINS[MAX_RELAYS] = {0};
size_t activeRelayCount = 0;  // Will be set during initialization
const unsigned long RELAY_PULSE_DURATION = 500;  // 500ms pulse duration

// Default relay pin configuration
const int DEFAULT_RELAY_PINS[] = {32, 33, 4, 16};
const int DEFAULT_RELAY_COUNT = 4;

// Button debounce variables
static unsigned long lastDebounceTime = 0;
static const unsigned long debounceDelay = 50;  // 50ms debounce time
static int lastButtonState = HIGH;


bool setDefaultRelayMapping() {
  Serial.println("Setting default relay mapping");
  
  // Create a temporary array with the default mapping
  int defaultMapping[DEFAULT_RELAY_COUNT];
  for (int i = 0; i < DEFAULT_RELAY_COUNT; i++) {
    defaultMapping[i] = DEFAULT_RELAY_PINS[i];
  }
  
  // Call the regular mapping function
  return setRelayMapping(defaultMapping, DEFAULT_RELAY_COUNT);
}

bool setRelayMapping(int* relayMapping, size_t mappingSize) {
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
    if (RELAY_PINS[i] > 0) {
      digitalWrite(RELAY_PINS[i], LOW);
      pinMode(RELAY_PINS[i], INPUT);
    }
  }
  
  // Clear existing array first
  for (int i = 0; i < MAX_RELAYS; i++) {
    RELAY_PINS[i] = 0;
  }
  
  // Copy new mapping values
  for (size_t i = 0; i < mappingSize; i++) {
    RELAY_PINS[i] = relayMapping[i];
  }
  
  // Store the active count
  activeRelayCount = mappingSize;
  
  // Initialize pins with new mapping
  setupRelays();
  
  // Log the new mapping
  Serial.printf("New relay mapping set with %d pins:\n", mappingSize);
  for (size_t i = 0; i < mappingSize; i++) {
    Serial.printf("  Relay %d: GPIO %d\n", i+1, RELAY_PINS[i]);
  }
  
  return true;
}

void setupResetButton() {
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Reset button initialized on pin " + String(RESET_BUTTON_PIN));
}

void setupRelays() {
  Serial.println("\nInitializing Relays...");
  
  // Initialize all relay pins as outputs
  for (int i = 0; i < MAX_RELAYS; i++) {
    if (RELAY_PINS[i] > 0) {  // Check for valid GPIO pin
      pinMode(RELAY_PINS[i], OUTPUT);
      digitalWrite(RELAY_PINS[i], LOW);
    }
  }
   
  Serial.println("Relays initialized in LOW state:");
  for (int i = 0; i < MAX_RELAYS; i++) {
    if (RELAY_PINS[i] > 0) {
      Serial.println("- Relay " + String(i + 1) + ": GPIO" + String(RELAY_PINS[i]) + " [LOW]");
    }
  }
  
  Serial.println("Pulse duration set to: " + String(RELAY_PULSE_DURATION) + "ms");
}

void pulseRelay(int relayPin) {
  // Check if this is a valid relay pin
  bool validPin = false;
  int relayIndex = -1;
  
  for (int i = 0; i < MAX_RELAYS; i++) {
    if (RELAY_PINS[i] == relayPin) {
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

bool isRelayPulsing(int relayPin) {
  // Simple implementation since we're using blocking delay()
  return false;
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