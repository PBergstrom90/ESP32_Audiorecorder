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

// DEBUG: Heap info print interval
unsigned long lastHeapPrintTime = 0;
const unsigned long heapPrintInterval = 5000; 
void printHeapInfo() {
  Serial.printf("Free heap: %d bytes\n", xPortGetFreeHeapSize());
}

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin()) {
    Serial.println("Failed to initialize LittleFS");
  }
  // Check for certificate existence
  if (!LittleFS.exists("/certs/ca.crt") || 
      !LittleFS.exists("/certs/esp32.crt") || 
      !LittleFS.exists("/certs/esp32.key")) {
      Serial.println("ERROR: Missing certificate file(s)");
      delay(1000);
      ESP.restart();
    }

  // Load certificates from LittleFS
  webSocketHandler.readCertFile("/certs/ca.crt", webSocketHandler.ca_cert_str);
  webSocketHandler.readCertFile("/certs/esp32.crt", webSocketHandler.client_cert_str);
  webSocketHandler.readCertFile("/certs/esp32.key", webSocketHandler.client_key_str);
  
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
      yield();
  }
  
  webServerHandler.connectToWiFi();
  webSocketHandler.begin();
  
  microphone.setup();
  webSocketHandler.setMicrophone(&microphone);
  
  webServerHandler.begin(&microphone, &webSocketHandler);
  Serial.println("--- SETUP COMPLETED ---");
}

void loop() {
  webSocketHandler.loop();
  esp_task_wdt_reset();  // Feed the watchdog

  // DEBUG: Print heap info
  if (millis() - lastHeapPrintTime >= heapPrintInterval) {
    lastHeapPrintTime = millis();
    printHeapInfo();
  }

}