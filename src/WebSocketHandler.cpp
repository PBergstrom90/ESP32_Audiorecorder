#include "WebSocketHandler.h"

void WebSocketHandler::begin() {
  webSocket.begin(HOST_SOCKET_IP, SOCKET_PORT_NUMBER, "/");
  webSocket.setReconnectInterval(RECONNECT_INTERVAL);
  webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
    webSocketEvent(type, payload, length);
  });
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

void WebSocketHandler::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("WebSocket connected to server.");
      webSocket.sendTXT("TYPE:ESP32"); // Send identification message
      break;

    case WStype_DISCONNECTED:
      Serial.println("WebSocket disconnected from server.");
      Serial.println("Reconnecting...");
      break;

    case WStype_TEXT: {
      String message = String((char *)payload);
      Serial.printf("Received TEXT: %s\n", message.c_str());

      if (message.startsWith("ACK:")) {
        Serial.println("Server acknowledged connection.");
      }
      break;
    }

    case WStype_BIN:
      Serial.printf("Received BINARY data of length: %d\n", length);
      break;

    case WStype_ERROR:
      Serial.println("WebSocket error occurred.");
      break;

    case WStype_PING:
      Serial.println("WebSocket PING received.");
      break;

    case WStype_PONG:
      Serial.println("WebSocket PONG received.");
      break;

    default:
      Serial.println("Unknown WebSocket event.");
      break;
  }
}