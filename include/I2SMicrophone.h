#ifndef I2S_MICROPHONE_H
#define I2S_MICROPHONE_H

#include <driver/i2s.h>
#include "WebSocketHandler.h"
#include "config.h"

enum class MicrophoneState {
    IDLE,
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
    bool getDeferredReset() const { return deferredReset; }
    void setDeferredReset(bool value) { deferredReset = value; }

private:
    float gainFactor = 0.3; 
    bool deferredReset = false;
    uint16_t recordDurationMs = RECORD_DURATION_MS; 
    WebSocketHandler *webSocketHandler;
    MicrophoneState currentState;
    TaskHandle_t recordingTaskHandle = NULL;
};

#endif // I2S_MICROPHONE_H