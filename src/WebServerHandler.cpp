#include "WebServerHandler.h"

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
  server.on("/start-record", HTTP_GET, [mic, ws](AsyncWebServerRequest *request) {
    mic->startRecording(ws, 0.3, RECORD_DURATION_MS); // Default 0.3 gain, 5 sec
    
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"recording started\"}");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });

  server.on("/set-gain", HTTP_GET, [](AsyncWebServerRequest *request) {
    String value;
    if (request->hasParam("value")) {
      value = request->getParam("value")->value();
      Serial.printf("Gain updated to: %s\n", value.c_str());

      // Send success response with CORS headers
      AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"gain updated\"}");
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "Content-Type");
      request->send(response);
    } else {
      // Handle missing parameter case
      AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{\"error\":\"Missing value parameter\"}");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    }
  });

  // Handle CORS preflight OPTIONS request
  server.on("/set-gain", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });

  server.begin();
  Serial.println("HTTP server started.");
}
