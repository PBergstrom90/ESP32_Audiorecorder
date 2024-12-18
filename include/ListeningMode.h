#ifndef LISTENING_MODE_H
#define LISTENING_MODE_H

#include <cmath>
#include "I2SMicrophone.h"
#include "WebSocketHandler.h"
#include "config.h"

class WebServerHandler;

class ListeningMode {
private:
    I2SMicrophone *microphone;
    WebSocketHandler *webSocketHandler;
    WebServerHandler *webServerHandler;
    float noiseThreshold;
    bool isListening;

public:
    ListeningMode(I2SMicrophone *mic, WebSocketHandler *ws, WebServerHandler *webServer);
    void startListening();
    void stopListening();
    void setThreshold(float threshold);

private:
    void processAudioData();
    float calculateRMS(const int32_t *samples, size_t count);
    static void listeningModeTask(void *parameter);
};

#endif // LISTENING_MODE_H