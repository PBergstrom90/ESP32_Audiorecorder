#include "ListeningMode.h"
#include "WebServerHandler.h"

ListeningMode::ListeningMode(I2SMicrophone *mic, WebSocketHandler *ws, WebServerHandler *webServer)
    : microphone(mic), webSocketHandler(ws), webServerHandler(webServer), noiseThreshold(4.0) {}

void ListeningMode::startListening() {
    if (listeningTaskHandle != NULL) {
        Serial.println("Listening task already running. Terminating...");
        vTaskDelete(listeningTaskHandle);
        listeningTaskHandle = NULL;
    }
    if (microphone->getState() != MicrophoneState::IDLE) {
        Serial.println("Cannot start listening. Microphone is not in IDLE state.");
        return;
    }
    microphone->reset();
    microphone->setState(MicrophoneState::LISTENING);
    webServerHandler->enableWiFiLightSleep();
    BaseType_t result = xTaskCreatePinnedToCore(listeningModeTask, "ListeningModeTask", 8192, this, 1, &listeningTaskHandle, 1);
    if (result != pdPASS) {
        Serial.println("Failed to create ListeningModeTask!");
        microphone->setState(MicrophoneState::ERROR);
        microphone->recoverFromError();
        return;
    } else {
        Serial.printf("Listening task created. Task handle: %p\n", listeningTaskHandle);
    }
    Serial.println("Listening mode started.");
}

void ListeningMode::stopListening() {
    if (microphone->getState() == MicrophoneState::RECORDING) {
        Serial.println("Recording in progress. Deferring reset until recording completes.");
        // Set a deferred reset flag
        microphone->setDeferredReset(true);
        return;
    }
    if (listeningTaskHandle != NULL) {
        vTaskDelete(listeningTaskHandle);
        listeningTaskHandle = NULL;
        Serial.println("Listening task terminated.");
    }
    microphone->reset();
    microphone->setState(MicrophoneState::IDLE);
    webServerHandler->disableWiFiLightSleep();
    Serial.println("Listening mode stopped.");
}

void ListeningMode::setThreshold(float threshold) {
    noiseThreshold = threshold;
    Serial.printf("Noise threshold set to: %.2f\n", threshold);
}

void ListeningMode::processAudioData() {
    int32_t sampleBuffer[64] = {0}; // Clear the buffer.
    microphone->warmUp();
    Serial.println("Microphone Listening...");
    while (microphone->getState() == MicrophoneState::LISTENING) {
    size_t bytesRead = microphone->readAudioData(sampleBuffer, sizeof(sampleBuffer));
    if (bytesRead == 0) {
        Serial.println("ERROR reading audio data. Transitioning to ERROR state.");
        microphone->setState(MicrophoneState::ERROR);
        microphone->recoverFromError();
        break;
    }
    if (bytesRead > 0) {
        float rms = 0;
        rms = calculateRMS(sampleBuffer, bytesRead / 4);
        if (rms > 1.0) {  // Only print interesting RMS values
            Serial.printf("Calculated RMS: %.2f\n", rms);
        }
        if (rms > noiseThreshold) {
            Serial.println("Noise detected. Starting recording...");
            webServerHandler->disableWiFiLightSleep();
            microphone->startRecording(webSocketHandler, 0.3, RECORD_DURATION_MS);

            // Wait for recording to finish
            while (MicrophoneState::RECORDING == microphone->getState()) {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }

            Serial.println("Recording completed.");
            microphone->reset();
            microphone->setState(MicrophoneState::LISTENING);
            webServerHandler->enableWiFiLightSleep();

            // Post-recording cool-down
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            if (microphone->getState() != MicrophoneState::LISTENING) {
                Serial.println("Listening mode terminated.");
                break;
            }
        }
    }
        vTaskDelay(50 / portTICK_PERIOD_MS); // Add a small delay to prevent CPU overuse
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
    while (listeningMode->microphone->getState() == MicrophoneState::LISTENING) {
        listeningMode->processAudioData();
    }
    Serial.println("Listening task terminated.");
    listeningMode->microphone->setState(MicrophoneState::IDLE);
    vTaskDelete(NULL);
}