#include "WebSocketHandler.h"

void WebSocketHandler::begin() {
  webSocket.beginSslWithCA(HOST_SOCKET_IP, SOCKET_PORT_NUMBER, "/ws", "/certs/ca.crt");
  webSocket.setReconnectInterval(RECONNECT_INTERVAL);
  webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
    webSocketEvent(type, payload, length);
  });
  Serial.println("WebSocket initialized.");
}

bool WebSocketHandler::setCACertFromFile(const char *path) {
    File caCertFile = LittleFS.open(path, "r");
    if (!caCertFile) {
        Serial.printf("Failed to open CA cert file: %s\n", path);
        return false;
    }

    String caCertString = caCertFile.readString();
    caCertFile.close();

    if (caCertString.length() == 0) {
        Serial.printf("CA cert file %s is empty.\n", path);
        return false;
    }

    Serial.println("Loaded CA Cert:");
    Serial.println(caCertString.substring(0, 100));

    return true;
}

void WebSocketHandler::loop() {
  webSocket.loop();
}

void WebSocketHandler::sendAudioData(const uint8_t *data, size_t length) {
  if (webSocket.isConnected()) {
    webSocket.sendBIN(data, length);
  }
}

void WebSocketHandler::sendStartMessage() {
  if (webSocket.isConnected()) {
    webSocket.sendTXT("START");
    Serial.println("Sent START message.");
  }
}

void WebSocketHandler::sendEndMessage() {
  if (webSocket.isConnected()) {
    webSocket.sendTXT("END");
    Serial.println("Sent END message.");
  }
}

void WebSocketHandler::sendModeMessage(const String &mode) {
  if (webSocket.isConnected()) {
    if (mode == "automatic" || mode == "manual") {
      String message = "MODE:" + mode;
      webSocket.sendTXT(message);
      Serial.println("Sent MODE message: " + message);
    } else {
      Serial.println("ERROR: Invalid mode. MODE message not sent.");
    }
  } else {
    Serial.println("WebSocket not connected. MODE message not sent.");
  }
}

void WebSocketHandler::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("WebSocket connected to server.");
      // Send identification message
      webSocket.sendTXT("TYPE:ESP32");
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