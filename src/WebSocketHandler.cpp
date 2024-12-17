#include "WebSocketHandler.h"

void WebSocketHandler::begin() {
  webSocket.begin("192.168.50.30", 5001, "/");
  webSocket.setReconnectInterval(5000);
  Serial.println("WebSocket initialized.");
}

void WebSocketHandler::loop() {
  webSocket.loop();
}

void WebSocketHandler::sendAudioData(const uint8_t *data, size_t length) {
  if (webSocket.isConnected()) {
    webSocket.sendBIN(data, length);
  }
}

void WebSocketHandler::sendEndMessage() {
  if (webSocket.isConnected()) {
    webSocket.sendTXT("END");
    Serial.println("Sent END message.");
  }
}