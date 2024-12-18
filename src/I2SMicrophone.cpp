#include "I2SMicrophone.h"

I2SMicrophone::I2SMicrophone() : recording(false){}

bool I2SMicrophone::isRecording() {
    return recording;
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
  if (recording) return;
  recording = true;
  
  gainFactor = gain;
  recordDurationMs = duration;
  webSocketHandler = webSocket;

  xTaskCreatePinnedToCore(recordingTask, "RecordingTask", 8192, this, 1, NULL, 1);
}

void I2SMicrophone::recordingTask(void *parameter) {
  I2SMicrophone *mic = static_cast<I2SMicrophone *>(parameter);
  uint8_t frameBuffer[128];
  int32_t sampleBuffer[64];

  Serial.println("Starting recording...");
  mic->warmUp();

  unsigned long start = millis();
  i2s_start(I2S_NUM_0);
  Serial.println("Recording...");
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
  mic->recording = false;
  vTaskDelete(NULL);
}

void I2SMicrophone::warmUp() {
  Serial.println("Warming up microphone...");
  delay(1000);  // Simple warm-up phase
  Serial.println("Warm up done. Microphone ready.");
}