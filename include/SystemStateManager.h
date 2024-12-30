#ifndef SYSTEMSTATEMANAGER_H
#define SYSTEMSTATEMANAGER_H

#include <FreeRTOS.h>
#include <freertos/semphr.h>
#include <Arduino.h>

enum class SystemMode {
    MANUAL,
    AUTOMATIC
};

class SystemStateManager {
private:
    SystemMode currentMode;
    SemaphoreHandle_t stateMutex; 

public:
    SystemStateManager();
    ~SystemStateManager();

    void setMode(SystemMode mode);
    SystemMode getMode();
    const char* getModeName();

    bool isAutomaticMode(); 
    bool isManualMode();
};

#endif // SYSTEMSTATEMANAGER_H
