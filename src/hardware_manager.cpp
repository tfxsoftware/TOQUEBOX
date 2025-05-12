#include "hardware_manager.h"
#include <SPIFFS.h>

// Hardware Configuration Constants
const int RESET_BUTTON_PIN = 13;  // Reset button on GPIO13

// Button debounce variables
static unsigned long lastDebounceTime = 0;
static const unsigned long debounceDelay = 50;  // 50ms debounce time
static int lastButtonState = HIGH;

void setupResetButton() {
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Reset button initialized on pin " + String(RESET_BUTTON_PIN));
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