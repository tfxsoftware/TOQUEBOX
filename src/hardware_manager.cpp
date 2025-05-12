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
  
  // Set all relays to LOW state for testing
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, LOW);
  
  Serial.println("Relays initialized in LOW state:");
  Serial.println("- Relay 1: GPIO" + String(RELAY_1_PIN) + " [LOW]");
  Serial.println("- Relay 2: GPIO" + String(RELAY_2_PIN) + " [LOW]");
  Serial.println("- Relay 3: GPIO" + String(RELAY_3_PIN) + " [LOW]");
  Serial.println("- Relay 4: GPIO" + String(RELAY_4_PIN) + " [LOW]");
  Serial.println("Pulse duration set to: " + String(RELAY_PULSE_DURATION) + "ms");
}

void pulseRelay(int relayPin) {
  digitalWrite(relayPin, HIGH);  // Turn on
  delay(500);                    // Wait 500ms
  digitalWrite(relayPin, LOW);   // Turn off
  Serial.printf("Pulsed relay on GPIO%d\n", relayPin);
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