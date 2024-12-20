#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <WebSocketsClient.h>
#include "config.h"

class WebSocketHandler {
public:
    void begin();
    void loop();
    void sendAudioData(const uint8_t *data, size_t length);
    void sendStartMessage();
    void sendEndMessage();
    void sendModeMessage(const String &mode);
    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

private:
    WebSocketsClient webSocket;
};

#endif // WEBSOCKET_HANDLER_H