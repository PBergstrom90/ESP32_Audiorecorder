#include "WebSocketHandler.h"

void WebSocketHandler::begin() {
  wifiClient.setCACert(ca_cert_str.c_str());
  wifiClient.setCertificate(client_cert_str.c_str());
  wifiClient.setPrivateKey(client_key_str.c_str());
  
  webSocket.beginSslWithCA(HOST_SOCKET_IP, SOCKET_PORT_NUMBER, "/ws/", ca_cert_str.c_str());
  webSocket.setReconnectInterval(RECONNECT_INTERVAL);
  webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
    webSocketEvent(type, payload, length);
  });
  Serial.println("WebSocket initialized.");
}

void WebSocketHandler::readCertFile(const char *path, String &dest) {
    File file = LittleFS.open(path, "r");
    if (!file) {
        Serial.printf("Failed to open file: %s\n", path);
        return;
    }

    dest = "";
    while (file.available()) {
        dest += char(file.read());
    }
    file.close();

    Serial.printf("Successfully read file: %s\n", path);
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