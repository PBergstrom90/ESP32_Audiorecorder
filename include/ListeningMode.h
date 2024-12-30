#ifndef LISTENING_MODE_H
#define LISTENING_MODE_H

#include <cmath>
#include "I2SMicrophone.h"
#include "WebSocketHandler.h"
#include "SystemStateManager.h"
#include "config.h"

class WebServerHandler;

class ListeningMode {
public:
    ListeningMode(I2SMicrophone *mic, WebSocketHandler *ws, WebServerHandler *webServer, SystemStateManager *stateManager);
    void startListening();
    void stopListening();
    void setThreshold(float threshold);

private:
    void processAudioData();
    float calculateRMS(const int32_t *samples, size_t count);
    static void listeningModeTask(void *parameter);
    I2SMicrophone *microphone;
    WebSocketHandler *webSocketHandler;
    WebServerHandler *webServerHandler;
    SystemStateManager *systemStateManager;
    TaskHandle_t listeningTaskHandle = NULL;
    float noiseThreshold;
};

#endif // LISTENING_MODE_H