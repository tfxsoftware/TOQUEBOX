#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include <Arduino.h>

// Hardware Configuration
extern const int RESET_BUTTON_PIN;

// Relay Configuration
#define MAX_RELAYS 16  // Maximum number of supported relays

// Relay mapping array
extern int RELAY_PINS[MAX_RELAYS];
extern size_t activeRelayCount;  // Number of currently active relays

extern const unsigned long RELAY_PULSE_DURATION;  // Duration of relay pulse in milliseconds

// Function declarations
void checkResetButton();
void setupResetButton();

// Relay functions
void setupRelays();
void pulseRelay(int relayPin);
bool isRelayPulsing(int relayPin);
bool setRelayMapping(int* relayMapping, size_t mappingSize);
bool setDefaultRelayMapping();  // Set default mapping as fallback

#endif // HARDWARE_MANAGER_H 