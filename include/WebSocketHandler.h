#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <WebSocketsClient.h>
#include <WifiClientSecure.h>
#include <LittleFS.h>
#include "config.h"

class WebSocketHandler {
public:
    void begin();
    void loop();
    void sendAudioData(const uint8_t *data, size_t length);
    void sendStartMessage();
    void sendEndMessage();
    void sendModeMessage(const String &mode);
    void readCertFile(const char *path, String &dest);

    String ca_cert_str;
    String client_cert_str;
    String client_key_str;

private:
    WebSocketsClient webSocket;
    WiFiClientSecure wifiClient;
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
};

#endif // WEBSOCKET_HANDLER_H