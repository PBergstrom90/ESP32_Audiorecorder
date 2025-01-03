#ifndef I2S_MICROPHONE_H
#define I2S_MICROPHONE_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "SystemStateManager.h"
#include "config.h"

// Forward declaration to avoid circular dependency
class WebSocketHandler;

// Enumeration for Microphone States
enum class MicrophoneState {
    IDLE,
    WARMUP,
    RECORDING,
    LISTENING,
    ERROR
};

class I2SMicrophone {
public:
    I2SMicrophone(SystemStateManager* mgr);
    MicrophoneState getState();
    SystemStateManager* systemStateManager;
    size_t readAudioData(int32_t *buffer, size_t bufferSize);
    float gainFactor;
    uint32_t recordDurationMs = RECORD_DURATION_MS;
    bool manualRecordingRequested; 
    void setup();
    void triggerRecording();
    void setState(MicrophoneState state);
    void reset();
    void warmUp();
    void recoverFromError();
    bool initializeHardware();
    void setGainFactor(float gain);
    float calculateRMS(const int32_t *samples, size_t count);

private:
    MicrophoneState currentState;
    const char* getStateName(MicrophoneState state);
};

#endif // I2S_MICROPHONE_H