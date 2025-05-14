#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include <Arduino.h>

class HardwareManager {
private:
    // Hardware Configuration Constants
    static constexpr int RESET_BUTTON_PIN = 13;  // Reset button on GPIO13
    
    // Relay Configuration
    static constexpr int MAX_RELAYS = 16;  // Maximum number of supported relays
    static constexpr unsigned long RELAY_PULSE_DURATION = 500;  // 500ms pulse duration
    
    // Default relay pin configuration
    static constexpr int DEFAULT_RELAY_PINS[4] = {32, 33, 4, 16};
    static constexpr int DEFAULT_RELAY_COUNT = 4;
    
    // Relay mapping array
    int relayPins[MAX_RELAYS] = {0};
    size_t activeRelayCount = 0;  // Number of currently active relays
    
    // Button debounce variables
    unsigned long lastDebounceTime = 0;
    static constexpr unsigned long debounceDelay = 50;  // 50ms debounce time
    int lastButtonState = HIGH;

public:
    // Constructor
    HardwareManager();
    
    // Button functions
    void setupResetButton();
    void checkResetButton();
    
    // Relay functions
    void setupRelays();
    void pulseRelay(int relayPin);
    bool isRelayPulsing(int relayPin);
    bool setRelayMapping(int* relayMapping, size_t mappingSize);
    bool setDefaultRelayMapping();
    
    // Accessor methods
    int getRelayPin(size_t index) const;
    size_t getActiveRelayCount() const;
};

// Global instance declaration
extern HardwareManager hardwareManager;

#endif // HARDWARE_MANAGER_H 