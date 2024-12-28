#include <Arduino.h>
#include <esp_task_wdt.h>
#include <LittleFS.h>
#include "I2SMicrophone.h"
#include "WebSocketHandler.h"
#include "WebServerHandler.h"
#include "ListeningMode.h"
#include "config.h"

I2SMicrophone microphone;           // I2S Microphone handler
WebSocketHandler webSocketHandler;  // WebSocket handler
WebServerHandler webServerHandler;  // Web server handler
ListeningMode listeningMode(&microphone, &webSocketHandler, &webServerHandler); // Listening mode handler

void setup() {
  Serial.begin(115200);
  LittleFS.begin();
  if (!LittleFS.begin()) {
    Serial.println("Failed to initialize LittleFS");
    while (true);
  }
  Serial.println("LittleFS initialized");
  
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
      yield();
  }
  
  webServerHandler.connectToWiFi();
  if (!webSocketHandler.setCACertFromFile("/certs/ca.crt")) {
    Serial.println("SSL ERROR: No valid CA cert loaded.");
  }
  webSocketHandler.begin();
  
  microphone.setup();
  
  webServerHandler.begin(&microphone, &webSocketHandler);
  Serial.println("--- SETUP COMPLETED ---");
}

void loop() {
  webSocketHandler.loop();
  esp_task_wdt_reset();  // Feed the watchdog
}