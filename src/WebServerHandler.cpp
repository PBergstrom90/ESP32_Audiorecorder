#include "WebServerHandler.h"

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

void WebServerHandler::enableWiFiLightSleep() {
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    Serial.println("Wi-Fi light sleep enabled.");
}

void WebServerHandler::disableWiFiLightSleep() {
    esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.println("Wi-Fi light sleep disabled.");
}

void WebServerHandler::begin(I2SMicrophone *mic, WebSocketHandler *ws) {
    server.on("/start-record", HTTP_GET, [this, mic, ws](AsyncWebServerRequest *request) {
        mic->startRecording(ws, 0.3, RECORD_DURATION_MS); // Default 0.3 gain, 5 sec

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"recording started\"}");
        addCORSHeaders(response);
        request->send(response);
    });

    server.on("/set-gain", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String value;
        if (request->hasParam("value")) {
            value = request->getParam("value")->value();
            Serial.printf("Gain updated to: %s\n", value.c_str());

            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"gain updated\"}");
            addCORSHeaders(response);
            request->send(response);
        } else {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{\"error\":\"Missing value parameter\"}");
            addCORSHeaders(response);
            request->send(response);
        }
    });

    server.on("/set-gain", HTTP_OPTIONS, [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200);
        addCORSHeaders(response);
        request->send(response);
    });

    server.on("/toggle-mode", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("mode")) {
            String mode = request->getParam("mode")->value();
            String responseMessage;

            if (mode == "automatic") {
                listeningMode.startListening();
                responseMessage = "{\"status\":\"Listening mode enabled.\"}";
            } else if (mode == "manual") {
                listeningMode.stopListening();
                responseMessage = "{\"status\":\"Manual mode enabled.\"}";
            } else {
                responseMessage = "{\"error\":\"Invalid mode.\"}";
                AsyncWebServerResponse *response = request->beginResponse(400, "application/json", responseMessage);
                addCORSHeaders(response);
                request->send(response);
                return;
            }

            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseMessage);
            addCORSHeaders(response);
            request->send(response);
        } else {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{\"error\":\"Mode parameter missing.\"}");
            addCORSHeaders(response);
            request->send(response);
        }
    });

    server.on("/toggle-mode", HTTP_OPTIONS, [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200);
        addCORSHeaders(response);
        request->send(response);
    });

    server.begin();
    Serial.println("HTTP server started.");
}