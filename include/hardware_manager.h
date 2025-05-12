#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include <Arduino.h>

// Hardware Configuration
extern const int RESET_BUTTON_PIN;

// Function declarations
void checkResetButton();
void setupResetButton();

#endif // HARDWARE_MANAGER_H 