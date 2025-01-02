#ifndef MICROPHONE_TASK_H
#define MICROPHONE_TASK_H

#include "I2SMicrophone.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include "WebSocketHandler.h"
#include "WebServerHandler.h"
#include "config.h"

class MicrophoneTask {
public:
    MicrophoneTask(I2SMicrophone* mic, WebSocketHandler* websocket, WebServerHandler* webServerHandler);
    void begin();
private:
    I2SMicrophone* mic;
    WebSocketHandler* websocket;
    WebServerHandler* webServerHandler;
    TaskHandle_t taskHandle;
    static void taskFunction(void* param);
    void run();
    void performRecording();
};

#endif // MICROPHONE_TASK_H
