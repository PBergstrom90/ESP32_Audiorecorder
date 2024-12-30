#include "SystemStateManager.h"

SystemStateManager::SystemStateManager() : currentMode(SystemMode::MANUAL) {
    stateMutex = xSemaphoreCreateMutex();
    if (stateMutex == NULL) {
        Serial.println("Failed to create state mutex!");
        while (1); // Halt system if mutex creation fails
    }
    Serial.println("SystemStateManager initialized. Default mode: MANUAL");
}

SystemStateManager::~SystemStateManager() {
    if (stateMutex != NULL) {
        vSemaphoreDelete(stateMutex);
    }
}

void SystemStateManager::setMode(SystemMode mode) {
    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(500))) {
        if (currentMode != mode) {
            currentMode = mode;
            Serial.printf("System mode changed to: %s\n", getModeName());
        }
        xSemaphoreGive(stateMutex);
    } else {
        Serial.println("Failed to acquire state mutex. Mode change aborted.");
    }
}

SystemMode SystemStateManager::getMode() {
    SystemMode mode;
    if (xSemaphoreTake(stateMutex, portMAX_DELAY)) {
        mode = currentMode;
        xSemaphoreGive(stateMutex);
    }
    return mode;
}

const char* SystemStateManager::getModeName() {
    switch (currentMode) {
        case SystemMode::MANUAL: return "MANUAL";
        case SystemMode::AUTOMATIC: return "AUTOMATIC";
        default: return "UNKNOWN";
    }
}

bool SystemStateManager::isAutomaticMode() {
    return getMode() == SystemMode::AUTOMATIC;
}

bool SystemStateManager::isManualMode() {
    return getMode() == SystemMode::MANUAL;
}