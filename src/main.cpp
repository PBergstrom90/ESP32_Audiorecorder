#include <Arduino.h>
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
  delay(1000);
  webServerHandler.connectToWiFi();
  webSocketHandler.begin();
  microphone.setup();
  webServerHandler.begin(&microphone, &webSocketHandler);

  Serial.println("--- SETUP COMPLETED ---");
}

void loop() {
  webSocketHandler.loop();
  delay(1);  // Prevent watchdog reset
}