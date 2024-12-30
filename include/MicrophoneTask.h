// MicrophoneTask.h
#ifndef MICROPHONE_TASK_H
#define MICROPHONE_TASK_H

#include "I2SMicrophone.h"
#include "WebSocketHandler.h"
#include "config.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class MicrophoneTask {
public:
    MicrophoneTask(I2SMicrophone* mic, WebSocketHandler* ws);
    void begin();
private:
    I2SMicrophone* mic;
    WebSocketHandler* ws;
    TaskHandle_t taskHandle;
    static void taskFunction(void* param);
    void run();
    void performRecording();
};

#endif // MICROPHONE_TASK_H
