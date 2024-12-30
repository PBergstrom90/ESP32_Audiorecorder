#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <esp_wifi.h>
#include "config.h"
#include "secrets.h"
#include "SystemStateManager.h"

class I2SMicrophone;
class WebSocketHandler;

class WebServerHandler {
public:
    WebServerHandler(SystemStateManager *stateManager);
    void addCORSHeaders(AsyncWebServerResponse *response);
    void connectToWiFi();
    void begin(I2SMicrophone *mic, WebSocketHandler *ws);
    void enableWiFiLightSleep();
    void disableWiFiLightSleep();

private:
    AsyncWebServer server{80};
    SystemStateManager *systemStateManager;
};

#endif // WEBSERVER_HANDLER_H