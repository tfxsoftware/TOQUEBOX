#include "hardware_manager.h"
#include <SPIFFS.h>

// Hardware Configuration Constants
const int RESET_BUTTON_PIN = 13;  // Reset button on GPIO13

// Relay Configuration Constants
const int RELAY_1_PIN = 32;
const int RELAY_2_PIN = 33;
const int RELAY_3_PIN = 4;
const int RELAY_4_PIN = 16;
const unsigned long RELAY_PULSE_DURATION = 500;  // 500ms pulse duration

// Button debounce variables
static unsigned long lastDebounceTime = 0;
static const unsigned long debounceDelay = 50;  // 50ms debounce time
static int lastButtonState = HIGH;

// Relay state tracking
struct RelayState {
  bool isPulsing;
  unsigned long pulseStartTime;
};

static RelayState relayStates[4] = {
  {false, 0},  // Relay 1
  {false, 0},  // Relay 2
  {false, 0},  // Relay 3
  {false, 0}   // Relay 4
};

// Helper function to get relay index from pin
static int getRelayIndex(int relayPin) {
  switch(relayPin) {
    case RELAY_1_PIN: return 0;
    case RELAY_2_PIN: return 1;
    case RELAY_3_PIN: return 2;
    case RELAY_4_PIN: return 3;
    default: return -1;
  }
}

void setupResetButton() {
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Reset button initialized on pin " + String(RESET_BUTTON_PIN));
}

void setupRelays() {
  Serial.println("\nInitializing Relays...");
  
  // Initialize all relay pins as outputs
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);
  
  // Ensure all relays start in OFF state (HIGH for active low relays)
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(RELAY_3_PIN, HIGH);
  digitalWrite(RELAY_4_PIN, HIGH);
  
  // Reset all relay states
  for(int i = 0; i < 4; i++) {
    relayStates[i].isPulsing = false;
    relayStates[i].pulseStartTime = 0;
  }
  
  Serial.println("Relays initialized in OFF state:");
  Serial.println("- Relay 1: GPIO" + String(RELAY_1_PIN) + " [OFF]");
  Serial.println("- Relay 2: GPIO" + String(RELAY_2_PIN) + " [OFF]");
  Serial.println("- Relay 3: GPIO" + String(RELAY_3_PIN) + " [OFF]");
  Serial.println("- Relay 4: GPIO" + String(RELAY_4_PIN) + " [OFF]");
  Serial.println("Pulse duration set to: " + String(RELAY_PULSE_DURATION) + "ms");
}

void pulseRelay(int relayPin) {
  int relayIndex = getRelayIndex(relayPin);
  if (relayIndex == -1) {
    Serial.println("Invalid relay pin!");
    return;
  }

  // Only start a new pulse if the relay isn't already pulsing
  if (!relayStates[relayIndex].isPulsing) {
    Serial.printf("Pulsing relay on GPIO%d\n", relayPin);
    digitalWrite(relayPin, LOW);  // Active LOW
    relayStates[relayIndex].isPulsing = true;
    relayStates[relayIndex].pulseStartTime = millis();
  }

  // Check if any active pulses need to be turned off
  unsigned long currentMillis = millis();
  for (int i = 0; i < 4; i++) {
    if (relayStates[i].isPulsing) {
      if (currentMillis - relayStates[i].pulseStartTime >= RELAY_PULSE_DURATION) {
        int pin;
        switch(i) {
          case 0: pin = RELAY_1_PIN; break;
          case 1: pin = RELAY_2_PIN; break;
          case 2: pin = RELAY_3_PIN; break;
          case 3: pin = RELAY_4_PIN; break;
        }
        digitalWrite(pin, HIGH);  // Turn off relay (active LOW)
        relayStates[i].isPulsing = false;
        Serial.printf("Relay on GPIO%d pulse completed\n", pin);
      }
    }
  }
}

bool isRelayPulsing(int relayPin) {
  int relayIndex = getRelayIndex(relayPin);
  if (relayIndex == -1) {
    return false;
  }
  return relayStates[relayIndex].isPulsing;
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