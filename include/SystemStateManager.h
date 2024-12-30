#ifndef SYSTEM_STATE_MANAGER_H
#define SYSTEM_STATE_MANAGER_H

#include <Arduino.h>

enum class SystemMode {
    AUTOMATIC,
    MANUAL
};

class SystemStateManager {
public:
    SystemStateManager();
    SystemMode getMode();
    void setMode(SystemMode mode);
private:
    SystemMode currentMode;
};

#endif // SYSTEM_STATE_MANAGER_H