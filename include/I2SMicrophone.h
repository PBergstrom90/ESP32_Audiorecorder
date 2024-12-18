#ifndef I2S_MICROPHONE_H
#define I2S_MICROPHONE_H

#include <driver/i2s.h>
#include "WebSocketHandler.h"
#include "config.h"

class I2SMicrophone {
public:
    I2SMicrophone();
    void setup();
    size_t readAudioData(int32_t *buffer, size_t bufferSize);
    void startRecording(WebSocketHandler *webSocket, float gain, uint32_t duration);
    bool isRecording();
    void reset();

private:
    void warmUp();
    static void recordingTask(void *parameter);
    bool recording;
    float gainFactor = 0.3;
    uint16_t recordDurationMs = RECORD_DURATION_MS;
    WebSocketHandler *webSocketHandler;
};

#endif // I2S_MICROPHONE_H