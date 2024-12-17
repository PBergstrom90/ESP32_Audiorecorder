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
    request->send(response);
  });

  server.begin();
  Serial.println("HTTP server started.");
}