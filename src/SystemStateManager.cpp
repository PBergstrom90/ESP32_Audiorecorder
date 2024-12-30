#include "SystemStateManager.h"
#include <Arduino.h>

SystemStateManager::SystemStateManager() 
    : currentMode(SystemMode::MANUAL) {
}

SystemMode SystemStateManager::getMode() {
    return currentMode;
}

void SystemStateManager::setMode(SystemMode mode) {
    if (currentMode != mode) {
        currentMode = mode;
        Serial.printf("System mode changed to: %s\n", 
                      mode == SystemMode::AUTOMATIC ? "AUTOMATIC" : "MANUAL");
    }
}