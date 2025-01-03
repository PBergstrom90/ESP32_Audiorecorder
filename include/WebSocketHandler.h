#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <LittleFS.h>
#include "config.h"
#include "secrets.h"
#include "I2SMicrophone.h"
#include "SystemStateManager.h"

class I2SMicrophone;

enum class WebSocketState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

class WebSocketHandler {
public:
    WebSocketHandler();
    String ca_cert_str;
    String client_cert_str;
    String client_key_str;
    void begin();
    void loop();
    void reconnect();
    void sendAudioData(const uint8_t *data, size_t length);
    void sendStartMessage();
    void sendEndMessage();
    void sendModeMessage(const String &mode);
    void readCertFile(const char *path, String &dest);

private:
    WebSocketsClient webSocket;
    WebSocketState currentState;
    I2SMicrophone* mic;
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    uint8_t fixedBuffer[WEBSOCKET_BUFFER_SIZE];
};

#endif // WEBSOCKET_HANDLER_H