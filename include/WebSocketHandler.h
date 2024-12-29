#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <WebSocketsClient.h>
#include <WifiClientSecure.h>
#include <LittleFS.h>
#include "I2SMicrophone.h"
#include "config.h"

class I2SMicrophone;

enum WebSocketState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

class WebSocketHandler {
public:
    WebSocketHandler();
    const char* getWebsocketStateName(WebSocketState state);
    WebSocketState getState() const;
    void begin();
    void loop();
    void readCertFile(const char *path, String &dest);
    void sendAudioData(const uint8_t *data, size_t length);
    void sendTextMessage(const String &message);
    void sendStartMessage();
    void sendEndMessage();
    void sendModeMessage(const String &mode);
    void setMicrophone(I2SMicrophone *mic) { microphone = mic; }

    String ca_cert_str;
    String client_cert_str;
    String client_key_str;

private:
    WebSocketsClient webSocket;
    WiFiClientSecure wifiClient;
    I2SMicrophone *microphone;
    WebSocketState currentState;
    uint8_t fixedBuffer[WEBSOCKET_BUFFER_SIZE];
    
    void setWebsocketState(WebSocketState newState);
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
};

#endif // WEBSOCKET_HANDLER_H