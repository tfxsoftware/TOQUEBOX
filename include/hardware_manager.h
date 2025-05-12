#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include <Arduino.h>

// Hardware Configuration
extern const int RESET_BUTTON_PIN;

// Relay Configuration
extern const int RELAY_1_PIN;  // GPIO32
extern const int RELAY_2_PIN;  // GPIO33
extern const int RELAY_3_PIN;  // GPIO4
extern const int RELAY_4_PIN;  // GPIO16

extern const unsigned long RELAY_PULSE_DURATION;  // Duration of relay pulse in milliseconds

// Function declarations
void checkResetButton();
void setupResetButton();

// Relay functions
void setupRelays();
void pulseRelay(int relayPin);
bool isRelayPulsing(int relayPin);

#endif // HARDWARE_MANAGER_H 