#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include <Arduino.h>

class HardwareManager {
private:
    static constexpr int RESET_BUTTON_PIN = 13;  
    
    static constexpr int MAX_RELAYS = 16;  
    static constexpr unsigned long RELAY_PULSE_DURATION = 500;  
    
    static constexpr int DEFAULT_RELAY_PINS[4] = {32, 33, 4, 16};
    static constexpr int DEFAULT_RELAY_COUNT = 4;
    
    int relayPins[MAX_RELAYS] = {0};
    size_t activeRelayCount = 0;  
    
    unsigned long lastDebounceTime = 0;
    static constexpr unsigned long debounceDelay = 50;  
    int lastButtonState = HIGH;

public:
    HardwareManager();
    
    void setupResetButton();
    void checkResetButton();
    
    void setupRelays();
    void pulseRelay(int relayPin);
    bool isRelayPulsing(int relayPin);
    bool setRelayMapping(int* relayMapping, size_t mappingSize);
    bool setDefaultRelayMapping();
    
    int getRelayPin(size_t index) const;
    size_t getActiveRelayCount() const;
};

extern HardwareManager hardwareManager;

#endif 
