#include "ListeningMode.h"
#include "WebServerHandler.h"

ListeningMode::ListeningMode(I2SMicrophone *mic, WebSocketHandler *ws, WebServerHandler *webServer, SystemStateManager *stateManager)
    : microphone(mic), webSocketHandler(ws), webServerHandler(webServer), systemStateManager(stateManager), noiseThreshold(NOISE_THRESHOLD) {}

void ListeningMode::startListening() {
    if (listeningTaskHandle != NULL) {
        if (microphone->getState() == MicrophoneState::LISTENING) {
            Serial.println("Listening task already running. No action taken.");
            return;
        } else {
            Serial.println("Stopping previous listening task...");
            stopListening();
        }
    }
    if (microphone->getState() != MicrophoneState::IDLE) {
        Serial.println("Cannot start listening. Microphone not IDLE.");
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
    } else {
        Serial.printf("Listening task created. Handle: %p\n", listeningTaskHandle);
    }

    Serial.println("Listening mode started.");
}

void ListeningMode::stopListening() {
    if (microphone->getState() == MicrophoneState::RECORDING) {
        Serial.println("Recording in progress. Deferring reset until recording completes.");
        microphone->reset();
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
        if (systemStateManager->getMode() == SystemMode::AUTOMATIC) {
                startListening(); // Retry listening
            }
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
            microphone->setState(MicrophoneState::IDLE);
            webServerHandler->disableWiFiLightSleep();
            microphone->startRecording(webSocketHandler, GAIN_VALUE, RECORD_DURATION_MS);

            // Wait for recording to finish
            while (MicrophoneState::RECORDING == microphone->getState()) {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }

            Serial.println("Recording completed.");
            if (systemStateManager->getMode() == SystemMode::AUTOMATIC) {
                microphone->setState(MicrophoneState::LISTENING);
                webServerHandler->enableWiFiLightSleep();
            }
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
    while (true) {
        if (listeningMode->systemStateManager->getMode() != SystemMode::AUTOMATIC) {
            Serial.println("System mode changed. Exiting listening mode.");
            break;
        }

        if (listeningMode->microphone->getState() == MicrophoneState::LISTENING) {
            listeningMode->processAudioData();
        } else {
            // Wait for the microphone to return to LISTENING
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    Serial.println("Listening task terminated.");
    vTaskDelete(NULL);
}
