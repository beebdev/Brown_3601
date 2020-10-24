// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Variables
bool buttonPress = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


// Notifies client using websocket protorcol
void notifyClients() {
  ws.textAll(String(buttonPress));
}

// Callback function that handles data we receive from client
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    /*
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
    */
  }
}

// Configures even listeners to handle asynchronous steps of WB protocol
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    // Client has logged on
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    // Client has disconnected
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    // Data packet recieved from client
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    // Reponse to a ping request
    case WS_EVT_PONG:
    // Error was received from the client
    case WS_EVT_ERROR:
      break;
  }
}

// Initialises websocket protocol
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Processor. (There are no placeholders in the html file)
String processor(const String& var){
  Serial.println(var);
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/style.css","text/css");
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
}