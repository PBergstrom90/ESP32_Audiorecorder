#include "WebSocketHandler.h"

WebSocketHandler::WebSocketHandler() : currentState(WebSocketState::DISCONNECTED) {}

WebSocketState WebSocketHandler::getState() const {
    return currentState;
}

const char* WebSocketHandler::getWebsocketStateName(WebSocketState state) {
    switch (state) {
        case WebSocketState::DISCONNECTED: return "DISCONNECTED";
        case WebSocketState::CONNECTING: return "CONNECTING";
        case WebSocketState::CONNECTED: return "CONNECTED";
        case WebSocketState::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void WebSocketHandler::setWebsocketState(WebSocketState newState) {
    if (currentState == newState) {
        return; 
    }
    currentState = newState;
    Serial.printf("WebSocket state changed to: %s\n", getWebsocketStateName(newState));

    switch (newState) {
        case WebSocketState::CONNECTING:
            Serial.println("Attempting to connect to WebSocket server...");
            break;

        case WebSocketState::CONNECTED:
            Serial.println("WebSocket connected.");
            break;

        case WebSocketState::DISCONNECTED:
            Serial.println("WebSocket disconnected.");
            break;

        case WebSocketState::ERROR:
            Serial.println("WebSocket encountered an error.");
            break;

        default:
            Serial.println("Unknown WebSocket state.");
            break;
    }
}

void WebSocketHandler::begin() {
    setWebsocketState(WebSocketState::CONNECTING);

    wifiClient.setCACert(ca_cert_str.c_str());
    wifiClient.setCertificate(client_cert_str.c_str());
    wifiClient.setPrivateKey(client_key_str.c_str());

    webSocket.beginSslWithCA(HOST_SOCKET_IP, SOCKET_PORT_NUMBER, "/ws/", ca_cert_str.c_str());
    webSocket.setReconnectInterval(RECONNECT_INTERVAL);
    webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
          webSocketEvent(type, payload, length);
      });
    Serial.println("WebSocket initialized.");
    setWebsocketState(WebSocketState::CONNECTED);
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
  static unsigned long lastRetryTime = 0;
  const unsigned long retryInterval = RECONNECT_INTERVAL;
  switch (currentState) {
        case CONNECTING:
            if (millis() - lastRetryTime >= retryInterval) {
                lastRetryTime = millis();
                webSocket.disconnect();
                Serial.println("Retrying WebSocket connection...");
                begin();
            }
            break;

        case CONNECTED:
            webSocket.loop();
            break;

        case DISCONNECTED:
            setWebsocketState(WebSocketState::CONNECTING);
            break;

        case ERROR:
            if (millis() - lastRetryTime >= retryInterval) {
              lastRetryTime = millis();
              setWebsocketState(WebSocketState::CONNECTING);
            }
            break;
    }
}

void WebSocketHandler::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      setWebsocketState(WebSocketState::CONNECTED);
      Serial.println("WebSocket connected to server.");
      webSocket.sendTXT("TYPE:ESP32");
      break;

    case WStype_DISCONNECTED:
      setWebsocketState(WebSocketState::DISCONNECTED);
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
      setWebsocketState(WebSocketState::ERROR);
      Serial.printf("WebSocket error occurred: Type=%d Length=%d\n", type, length);
      break;

    case WStype_PING:
      Serial.println("WebSocket PING received.");
      if (microphone && microphone->getState() == MicrophoneState::RECORDING || microphone->getState() == MicrophoneState::WARMUP) {
        Serial.println("Ignoring WebSocket PING during recording.");
        return;
      } 
      break;

    case WStype_PONG:
      Serial.println("WebSocket PONG received.");
      break;

    default:
      Serial.println("Unknown WebSocket event.");
      break;
  }
}

void WebSocketHandler::sendAudioData(const uint8_t *data, size_t length) {
  if (length > WEBSOCKET_BUFFER_SIZE) {
        Serial.println("ERROR: Payload size exceeds WebSocket buffer size.");
        return;
    }
  if (currentState == WebSocketState::CONNECTED) {
    memcpy(fixedBuffer, data, length);
    webSocket.sendBIN(fixedBuffer, length);
  }
}

void WebSocketHandler::sendStartMessage() {
  if (currentState == WebSocketState::CONNECTED) {
    webSocket.sendTXT("START");
    Serial.println("Sent START message.");
  }
}

void WebSocketHandler::sendEndMessage() {
  if (currentState == WebSocketState::CONNECTED) {
    webSocket.sendTXT("END");
    Serial.println("Sent END message.");
  }
}

void WebSocketHandler::sendModeMessage(const String &mode) {
  if (currentState == WebSocketState::CONNECTED) {
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