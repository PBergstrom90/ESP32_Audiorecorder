#include "WebSocketHandler.h"
#include <LittleFS.h>
#include "config.h"
#include "secrets.h"

WebSocketHandler::WebSocketHandler() 
    : currentState(WebSocketState::DISCONNECTED) {}

void WebSocketHandler::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            currentState = WebSocketState::CONNECTED;
            Serial.println("WebSocket connected to server.");
            webSocket.sendTXT("TYPE:ESP32");
            break;

        case WStype_DISCONNECTED:
            currentState = WebSocketState::DISCONNECTED;
            Serial.println("WebSocket disconnected from server.");
            Serial.println("Reconnecting...");
            reconnect();
            break;

        case WStype_TEXT: {
            String message = String((char *)payload);
            Serial.printf("Received TEXT: %s\n", message.c_str());

            if (message.startsWith("ACK:")) {
                Serial.println("Server acknowledged connection.");
            } else if (message.startsWith("MODE:")) {
                Serial.printf("Server set mode to: %s\n", message.substring(5).c_str());
            }
            break;
        }

        case WStype_BIN:
            Serial.printf("Received BINARY data of length: %d\n", length);
            break;

        case WStype_ERROR:
            currentState = WebSocketState::ERROR;
            Serial.printf("WebSocket error occurred: Type=%d Length=%d\n", type, length);
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

void WebSocketHandler::begin() {
    // Read certificates from LittleFS
    readCertFile("/certs/ca.crt", ca_cert_str);
    readCertFile("/certs/esp32.crt", client_cert_str);
    readCertFile("/certs/esp32.key", client_key_str);

    // Initialize WebSocket
    webSocket.beginSslWithCA(HOST_SOCKET_IP, SOCKET_PORT_NUMBER, "/ws/", ca_cert_str.c_str());
    webSocket.setReconnectInterval(RECONNECT_INTERVAL);
    webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        webSocketEvent(type, payload, length);
    });

    currentState = WebSocketState::CONNECTING;
    Serial.println("WebSocket initialized.");
}

void WebSocketHandler::reconnect() {
    webSocket.disconnect();
    webSocket.beginSslWithCA(HOST_SOCKET_IP, SOCKET_PORT_NUMBER, "/ws/", ca_cert_str.c_str());
    webSocket.setReconnectInterval(RECONNECT_INTERVAL);
    webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        webSocketEvent(type, payload, length);
    });
    currentState = WebSocketState::CONNECTING;
    Serial.println("WebSocket reconnecting...");
}

void WebSocketHandler::loop() {
    webSocket.loop();
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