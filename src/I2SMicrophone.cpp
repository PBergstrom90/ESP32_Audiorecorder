#include "I2SMicrophone.h"

I2SMicrophone::I2SMicrophone() : currentState(MicrophoneState::IDLE) {}

MicrophoneState I2SMicrophone::getState() {
    return currentState;
}

void I2SMicrophone::setState(MicrophoneState state) {
    currentState = state;
    Serial.printf("Microphone state changed to: %s\n", getStateName(state));
}

const char* I2SMicrophone::getStateName(MicrophoneState state) {
    switch (state) {
        case MicrophoneState::IDLE: return "IDLE";
        case MicrophoneState::WARMUP: return "WARMUP";
        case MicrophoneState::RECORDING: return "RECORDING";
        case MicrophoneState::LISTENING: return "LISTENING";
        case MicrophoneState::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void I2SMicrophone::setup() {
  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 64,
      .use_apll = false,
  };

  esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("I2S driver install failed: %s\n", esp_err_to_name(err));
    while (1);
  }

  const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = -1,
      .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
  Serial.println("I2S microphone initialized.");
  setState(MicrophoneState::IDLE);
}

size_t I2SMicrophone::readAudioData(int32_t *buffer, size_t bufferSize) {
  size_t bytesRead = 0;
  esp_err_t err = i2s_read(I2S_PORT, buffer, bufferSize, &bytesRead, portMAX_DELAY);
  if (err != ESP_OK) {
    Serial.printf("I2S read failed: %s\n", esp_err_to_name(err));
  }
  return bytesRead;
}

void I2SMicrophone::startRecording(WebSocketHandler *webSocket, float gain, uint32_t duration) {
    if (currentState != MicrophoneState::IDLE && currentState != MicrophoneState::LISTENING) {
        Serial.println("Cannot start recording. Microphone is not in an IDLE or LISTENING state.");
        return;
    }
    if (currentState == MicrophoneState::RECORDING) {
      Serial.println("Already recording. Cannot start a new recording.");
      return;
    }
    if (recordingTaskHandle != NULL) {
        Serial.println("Previous recording task is still active. Cannot start a new recording.");
        return;
    }

    gainFactor = gain;
    recordDurationMs = duration;
    webSocketHandler = webSocket;

    recordingTaskHandle = NULL;
    BaseType_t result = xTaskCreatePinnedToCore(recordingTask, "RecordingTask", 12288, this, 3, &recordingTaskHandle, 0);
    if (result != pdPASS) {
        Serial.println("Failed to create recording task!");
        setState(MicrophoneState::ERROR);
        recoverFromError();
        return;
    }
}

void I2SMicrophone::recordingTask(void *parameter) {
    I2SMicrophone *mic = static_cast<I2SMicrophone *>(parameter);
    mic->reset();
    uint8_t frameBuffer[256];
    int32_t sampleBuffer[128];
    if (frameBuffer == nullptr) {
        Serial.println("ERROR: Failed to allocate memory for frameBuffer.");
        return;
    } else if (sampleBuffer == nullptr) {
        Serial.println("ERROR: Failed to allocate memory for sampleBuffer.");
        return;
    }

    Serial.println("Starting recording...");
    mic->setState(MicrophoneState::WARMUP);
    mic->warmUp();

    mic->setState(MicrophoneState::RECORDING);
    unsigned long start = millis();
    i2s_start(I2S_NUM_0);
    Serial.println("Recording...");
    mic->webSocketHandler->sendStartMessage();

    while (millis() - start < mic->recordDurationMs) {
        size_t bytesRead;
        if (i2s_read(I2S_NUM_0, sampleBuffer, sizeof(sampleBuffer), &bytesRead, portMAX_DELAY) == ESP_OK) {
            int idx = 0;
            for (int i = 0; i < bytesRead / 4; i++) {
                int16_t sample16 = (sampleBuffer[i] >> 8) * mic->gainFactor;
                memcpy(&frameBuffer[idx], &sample16, sizeof(sample16));
                idx += 2;
            }
            mic->webSocketHandler->sendAudioData(frameBuffer, idx);
        }
    }

    i2s_stop(I2S_NUM_0);
    mic->webSocketHandler->sendEndMessage();
    Serial.println("Recording finished.");
    
    mic->reset();
    mic->setState(MicrophoneState::IDLE);
    mic->recordingTaskHandle = NULL;
    vTaskDelete(NULL);
}

void I2SMicrophone::recoverFromError() {
    if (getState() != MicrophoneState::ERROR) {
        Serial.println("Recovery not required. Microphone is not in ERROR state.");
        return;
    }

    Serial.println("Attempting to recover from ERROR state...");
    reset();

    if (initializeHardware()) { 
        setState(MicrophoneState::IDLE);
        Serial.println("Recovery successful. Microphone is now IDLE.");
    } else {
        Serial.println("Recovery failed. Restarting the device...");
        delay(3000);
        esp_restart();
    }
}

bool I2SMicrophone::initializeHardware() {
    int32_t sampleBuffer[64] = {0};
    size_t bytesRead = readAudioData(sampleBuffer, sizeof(sampleBuffer));

    if (bytesRead > 0) {
        Serial.println("Hardware initialized successfully.");
        return true; 
    } else {
        Serial.println("Hardware initialization failed.");
        return false;
    }
}

void I2SMicrophone::warmUp() {
    Serial.println("Warming up microphone...");

    int32_t sampleBuffer[64] = {0};
    const uint8_t WARM_UP_PASSES = 10;

    for (int i = 0; i < WARM_UP_PASSES; i++) {
        memset(sampleBuffer, 0, sizeof(sampleBuffer));
        size_t bytesRead = readAudioData(sampleBuffer, sizeof(sampleBuffer));
        Serial.printf("Warm-up pass %d: %d bytes read\n", i + 1, bytesRead);
        if (bytesRead == 0) {
            Serial.println("ERROR: Warm-up failed. Hardware not responding.");
            setState(MicrophoneState::ERROR);
            return;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    Serial.println("Warm-up phase completed. Microphone ready.");
}

void I2SMicrophone::reset() {
    i2s_stop(I2S_NUM_0);
    i2s_start(I2S_NUM_0);
    Serial.println("I2S peripheral reset.");
}