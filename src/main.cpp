#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
#include <credentials.h>
#include <variables.h>

const char *ssid = BOCIASAN_WIFI_SSID;
const char *password = BOCIASAN_WIFI_PASS;

// Webserver
AsyncWebServer server(80);
//Websocket
AsyncWebSocket ws("/ws");

JSONVar state;

const int LED_PIN = LED_BUILTIN;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

String getInitialStateString(){
  state["mode"] = mode;
  state["status"] = status;
  state["target"] = round(target * 100) / 100.0;;
  state["current_temp"] = round(current_temp * 100) / 100.0;
  state["scene"] = scene;
  state["esp_time"] = esp_time;
  state["ssid"] = WiFi.SSID();
  state["rssi"] = WiFi.RSSI();
  state["weekly_stats"][0][0] = weekly_stats[0][0];
  state["weekly_stats"][0][1] = weekly_stats[0][1];
  state["weekly_stats"][1][0] = weekly_stats[1][0];
  state["weekly_stats"][1][1] = weekly_stats[1][1];
  state["weekly_stats"][2][0] = weekly_stats[2][0];
  state["weekly_stats"][2][1] = weekly_stats[2][1];
  state["weekly_stats"][3][0] = weekly_stats[3][0];
  state["weekly_stats"][3][1] = weekly_stats[3][1];
  state["city"] = city;
  state["reponse_type"] = String("get");
  state["response_value"] = String("initial_state");
  // state["sensors"][0] = WiFi.RSSI();

  // state[""] = ;
  // state[""] = ;
  return JSON.stringify(state);
}

void notifyClients(String json_string) {
  ws.textAll(json_string);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, AsyncWebSocketClient *client) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    // message = (char*)data;
    // Serial.println(message);
    JSONVar payload = JSON.parse((char*)data);
    // payload["counter"] = (int)payload["counter"]+1;
    // String jsonString = JSON.stringify(payload);
    // ws.textAll(jsonString);

    if (payload["type"] == String("get")){
      if (payload["value"] == String("initial_state"))
      client->text(getInitialStateString());
    } 
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      // client->text("{\"counter\":\"777\"}");
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len, client);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}


void setup()
{

  Serial.begin(115200);
  Serial.println("Starting the LittleFS Webserver..");

  // Begin LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Connect to WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // LED PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Route for root index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    { request->send(LittleFS, "/index.html", "text/html"); });
  server.onNotFound(notFound);
  server.begin();
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.serveStatic("/", LittleFS, "/");
}

void loop()
{
  ws.cleanupClients();
}