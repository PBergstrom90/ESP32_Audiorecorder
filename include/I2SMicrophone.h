#ifndef I2S_MICROPHONE_H
#define I2S_MICROPHONE_H

#include <driver/i2s.h>
#include "WebSocketHandler.h"
#include "SystemStateManager.h"
#include "config.h"

class WebSocketHandler;
class SystemStateManager;

enum class MicrophoneState {
    IDLE,
    WARMUP,
    RECORDING,
    LISTENING,
    ERROR
};

class I2SMicrophone {
public:
    I2SMicrophone();
    MicrophoneState getState();
    void setState(MicrophoneState state);
    const char* getStateName(MicrophoneState state);
    void setup();
    size_t readAudioData(int32_t *buffer, size_t bufferSize);
    void startRecording(WebSocketHandler *webSocket, float gain, uint32_t duration);
    static void recordingTask(void *parameter);
    void recoverFromError();
    void warmUp();
    void reset();
    bool initializeHardware();

private:
    float gainFactor = GAIN_VALUE; 
    SemaphoreHandle_t stateMutex;
    SemaphoreHandle_t i2sLock;
    uint16_t recordDurationMs = RECORD_DURATION_MS; 
    SystemStateManager *systemStateManager;
    WebSocketHandler *webSocketHandler;
    MicrophoneState currentState;
    TaskHandle_t recordingTaskHandle = NULL;
};

#endif // I2S_MICROPHONE_H