#include <Arduino.h>
#include <esp_task_wdt.h>
#include <LittleFS.h>
#include "I2SMicrophone.h"
#include "WebSocketHandler.h"
#include "WebServerHandler.h"
#include "SystemStateManager.h"
#include "MicrophoneTask.h"

SystemStateManager systemStateManager; // System state manager
I2SMicrophone microphone(&systemStateManager); // I2S Microphone handler
WebSocketHandler webSocketHandler;  // WebSocket handler
WebServerHandler webServerHandler(&systemStateManager);  // Web server handler
MicrophoneTask microphoneTask(&microphone, &webSocketHandler, &webServerHandler);  // Microphone task

void setup() {
    Serial.begin(115200);
    Serial.println("--- SETUP STARTED ---");

    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("Failed to initialize LittleFS");
        while (1);
    }

    // Check for certificate files
    if (!LittleFS.exists("/certs/ca.crt") || 
        !LittleFS.exists("/certs/esp32.crt") || 
        !LittleFS.exists("/certs/esp32.key")) {
        Serial.println("ERROR: Missing certificate file(s)");
        delay(1000);
        ESP.restart();
    }

    webSocketHandler.begin();
    webServerHandler.connectToWiFi();
    microphone.setup();
    webServerHandler.begin(&microphone, &webSocketHandler);
    esp_task_wdt_init(20, true);
    esp_task_wdt_add(NULL);
    microphoneTask.begin();
    Serial.println("--- SETUP COMPLETED ---");
}

void loop() {
    webSocketHandler.loop();
    esp_task_wdt_reset();  // Feed the watchdog
}