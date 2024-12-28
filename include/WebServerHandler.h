#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H

#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include "I2SMicrophone.h"
#include "WebSocketHandler.h"
#include "ListeningMode.h"
#include "config.h"
#include "secrets.h"

extern ListeningMode listeningMode;

class WebServerHandler {
public:
    void addCORSHeaders(AsyncWebServerResponse *response);
    void connectToWiFi();
    void begin(I2SMicrophone *mic, WebSocketHandler *ws);
    void enableWiFiLightSleep();
    void disableWiFiLightSleep();

private:
    AsyncWebServer server{80};
};

#endif // WEBSERVER_HANDLER_H