#ifndef I2S_MICROPHONE_H
#define I2S_MICROPHONE_H

#include <driver/i2s.h>
#include "WebSocketHandler.h"
#include "config.h"

class I2SMicrophone {
public:
    I2SMicrophone();
    void setup();
    void startRecording(WebSocketHandler *webSocket, float gain, uint32_t duration);

private:
    void warmUp();
    static void recordingTask(void *parameter);

    float gainFactor = 0.3;
    uint16_t recordDurationMs = 5000;  // Default recording duration
    WebSocketHandler *webSocketHandler;
};

#endif