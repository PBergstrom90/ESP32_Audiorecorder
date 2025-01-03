#include "WebServerHandler.h"

WebServerHandler::WebServerHandler(SystemStateManager *stateManager) 
    : systemStateManager(stateManager) {}

void WebServerHandler::addCORSHeaders(AsyncWebServerResponse *response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

void WebServerHandler::connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

void WebServerHandler::begin(I2SMicrophone *mic, WebSocketHandler *ws) {
    server.on("/start-record", HTTP_GET, [this, mic, ws](AsyncWebServerRequest *request) {
        mic->triggerRecording(); 

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"recording started\"}");
        addCORSHeaders(response);
        request->send(response);
    });

    server.on("/set-gain", HTTP_GET, [this, mic](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            String value = request->getParam("value")->value();
            float gain = value.toFloat();
            mic->setGainFactor(gain);
            Serial.printf("Gain updated to: %.2f\n", gain);

            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"gain updated\"}");
            addCORSHeaders(response);
            request->send(response);
        } else {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{\"error\":\"Missing value parameter\"}");
            addCORSHeaders(response);
            request->send(response);
        }
    });

    // Handle CORS preflight for /set-gain.
    server.on("/set-gain", HTTP_OPTIONS, [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200);
        addCORSHeaders(response);
        request->send(response);
    });

    // Endpoint to toggle mode between AUTOMATIC and MANUAL.
    server.on("/toggle-mode", HTTP_GET, [this, ws, mic](AsyncWebServerRequest *request) {
        if (request->hasParam("mode")) {
            String mode = request->getParam("mode")->value();
            String responseMessage;

            if (mode == "automatic") {
                systemStateManager->setMode(SystemMode::AUTOMATIC);
                responseMessage = "MODE:automatic";
                ws->sendModeMessage(mode);
                Serial.println("System set to AUTOMATIC mode.");
            } else if (mode == "manual") {
                systemStateManager->setMode(SystemMode::MANUAL);
                responseMessage = "MODE:manual";
                ws->sendModeMessage(mode);
                Serial.println("System set to MANUAL mode.");
            } else {
                responseMessage = "ERROR:Invalid mode";
                AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", responseMessage);
                addCORSHeaders(response);
                request->send(response);
                Serial.println("Invalid mode parameter received.");
                return;
            }

            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", responseMessage);
            addCORSHeaders(response);
            request->send(response);
        } else {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{\"error\":\"Mode parameter missing.\"}");
            addCORSHeaders(response);
            request->send(response);
            Serial.println("Mode parameter missing.");
        }
    });

    // Handle CORS preflight for /toggle-mode.
    server.on("/toggle-mode", HTTP_OPTIONS, [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200);
        addCORSHeaders(response);
        request->send(response);
    });

    server.begin();
    Serial.println("HTTP server started.");
}