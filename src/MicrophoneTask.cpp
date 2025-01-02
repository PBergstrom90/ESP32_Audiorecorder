#include "MicrophoneTask.h"
#include <esp_task_wdt.h>

MicrophoneTask::MicrophoneTask(I2SMicrophone* micPtr, WebSocketHandler* wsPtr, WebServerHandler* wsHandler)
    : mic(micPtr), websocket(wsPtr), webServerHandler(wsHandler), taskHandle(NULL) {}

void MicrophoneTask::begin() {
    xTaskCreatePinnedToCore(
        MicrophoneTask::taskFunction,       // Task function
        "MicControllerTask",                // Task name
        12288,                              // Stack size (adjust as needed)
        this,                               // Parameter (pointer to this instance)
        2,                                  // Priority
        &taskHandle,                        // Task handle
        1                                   // Core ID (0 or 1)
    );

    if (taskHandle != NULL) {
        Serial.println("Microphone Controller Task created successfully.");
    } else {
        Serial.println("ERROR: Failed to create Microphone Controller Task.");
    }
}

void MicrophoneTask::taskFunction(void* param) {
    MicrophoneTask* task = static_cast<MicrophoneTask*>(param);
    task->run();
    vTaskDelete(NULL); // Delete the task if run() exits
}

void MicrophoneTask::run() {
    Serial.println("Microphone Controller Task started.");
    while (true) {
        SystemMode currentMode = mic->systemStateManager->getMode();
        if (currentMode == SystemMode::AUTOMATIC) {
            if (mic->getState() != MicrophoneState::LISTENING) {
                mic->setState(MicrophoneState::LISTENING);
                Serial.println("Microphone is now LISTENING (AUTOMATIC mode).");
            }
            int32_t sampleBuffer[64] = {0};
            size_t bytesRead = mic->readAudioData(sampleBuffer, sizeof(sampleBuffer));
            if (bytesRead > 0) {
                float rms = mic->calculateRMS(sampleBuffer, bytesRead / 4);
                if (rms > 1.0f) {
                    Serial.printf("Calculated RMS: %.2f\n", rms);
                }

                if (rms > NOISE_THRESHOLD) {
                    Serial.println("Noise threshold exceeded. Starting recording...");
                    performRecording();
                }
            }
        } else { // MANUAL mode
            if (mic->manualRecordingRequested) {
                mic->manualRecordingRequested = false;
                Serial.println("Manual recording requested. Starting recording...");
                performRecording();
            }
                vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    // Feed the watchdog
    esp_task_wdt_reset();
}

void MicrophoneTask::performRecording() {
    Serial.println("Starting recording...");
    mic->setState(MicrophoneState::WARMUP);
    mic->warmUp();
    mic->setState(MicrophoneState::RECORDING);
    i2s_start(I2S_NUM_0);
    Serial.println("Recording...");
    websocket->sendStartMessage();

    unsigned long start = millis();
    Serial.println("Recording loop started.");
    while (millis() - start < mic->recordDurationMs) {
        unsigned long elapsed = millis() - start;
        int32_t sampleBuffer[800] = {0};
        uint8_t frameBuffer[1600] = {0};
        size_t bytesRead;
        if (i2s_read(I2S_NUM_0, sampleBuffer, sizeof(sampleBuffer), &bytesRead, portMAX_DELAY) == ESP_OK) {
            int idx = 0;
            for (int i = 0; i < bytesRead / 4; i++) {
                int16_t sample16 = (sampleBuffer[i] >> 8) * mic->gainFactor;
                memcpy(&frameBuffer[idx], &sample16, sizeof(sample16));
                idx += 2;
            }
            websocket->sendAudioData(frameBuffer, idx);
            vTaskDelay(5 / portTICK_PERIOD_MS); // Delay to avoid overloading the WebSocket
        }
    }
    i2s_stop(I2S_NUM_0);
    websocket->sendEndMessage();
    Serial.println("Recording finished.");
    mic->reset();
    mic->setState(MicrophoneState::IDLE);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay to avoid immediate recording
}