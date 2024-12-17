#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H

#include <ESPAsyncWebServer.h>
#include "I2SMicrophone.h"
#include "WebSocketHandler.h"
#include "config.h"
#include "secrets.h"

class WebServerHandler {
public:
    void connectToWiFi();
    void begin(I2SMicrophone *mic, WebSocketHandler *ws);

private:
    AsyncWebServer server{80}; // Initialize with port number 80
};

#endif