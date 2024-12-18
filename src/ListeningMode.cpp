#include "ListeningMode.h"
#include "WebServerHandler.h"

ListeningMode::ListeningMode(I2SMicrophone *mic, WebSocketHandler *ws, WebServerHandler *webServer)
    : microphone(mic), webSocketHandler(ws), webServerHandler(webServer), noiseThreshold(0.5), isListening(false) {}

void ListeningMode::startListening() {
    if (!isListening) {
        isListening = true;
        webServerHandler->enableWiFiLightSleep();
        xTaskCreatePinnedToCore(listeningModeTask, "ListeningModeTask", 8192, this, 1, NULL, 1);
        Serial.println("Listening mode started.");
        } else {
        Serial.println("Listening mode already active.");
    }
}

void ListeningMode::stopListening() {
    isListening = false;
    Serial.println("Listening mode stopped.");
    webServerHandler->disableWiFiLightSleep();
}

void ListeningMode::setThreshold(float threshold) {
    noiseThreshold = threshold;
    Serial.printf("Noise threshold set to: %.2f\n", noiseThreshold);
}

void ListeningMode::processAudioData() {
    int32_t sampleBuffer[64];

    while (isListening) {
        size_t bytesRead = microphone->readAudioData(sampleBuffer, sizeof(sampleBuffer));
        if (bytesRead > 0) {
            float rms = calculateRMS(sampleBuffer, bytesRead / 4);
            Serial.printf("Calculated RMS: %.2f\n", rms);

            if (rms > noiseThreshold) {
                Serial.printf("Noise detected! RMS: %.2f\n", rms);
                webServerHandler->disableWiFiLightSleep();

                microphone->startRecording(webSocketHandler, 0.3, RECORD_DURATION_MS);

                // Wait for recording to finish
                while (microphone->isRecording()) {
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
                if (isListening) {
                    Serial.println("Recording completed, resuming listening mode.");
                    return;
                } else {
                    Serial.println("Recording completed, staying idle in manual mode.");
                }
            }
        }
    }
}

float ListeningMode::calculateRMS(const int32_t *samples, size_t count) {
    double sum = 0;
    for (size_t i = 0; i < count; i++) {
        float sample = (float)(samples[i] >> 8) / INT16_MAX;
        sum += sample * sample;
    }
    return sqrt(sum / count);
}

void ListeningMode::listeningModeTask(void *parameter) {
    ListeningMode *listeningMode = static_cast<ListeningMode *>(parameter);
    listeningMode->processAudioData();
    vTaskDelete(NULL);
}