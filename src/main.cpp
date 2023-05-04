#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
// #include <Arduino_JSON.h>
#include <ArduinoJson.h>
#include <credentials.h>
#include <variables.h>

const char *ssid = BOCIASAN_WIFI_SSID;
const char *password = BOCIASAN_WIFI_PASS;

// Webserver
AsyncWebServer server(80);
//Websocket
AsyncWebSocket ws("/ws");

// JSONVar state;
DynamicJsonDocument state(1024);
StaticJsonDocument<256> response;
StaticJsonDocument<256> item;

const int LED_PIN = LED_BUILTIN;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

String getInitialStateString(){
  state["mode"] = mode;
  state["status"] = status;
  state["target"] = round(target * 100) / 100.0;
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
  state["rtype"] = "get";
  state["rparam"] = "initial_state";
  // state["sensors"][0] = WiFi.RSSI();

  // state[""] = ;
  // state[""] = ;
  String res;
  serializeJson(state, res);
  return res;
  // return JSON.stringify(state);
}

String getVariableString(String rparam){
  // JSONVar response;
  response["rtype"] = "get";

  if (rparam == "mode"){
    response["rparam"] = "mode";
    response["rvalue"] = mode;
  } else 
  if (rparam == "status"){
    response["rparam"] = "status";
    response["rvalue"] = status;
  }else 
  if (rparam == "scene"){
    response["rparam"] = "scene";
    response["rvalue"] = scene;
  }
  // String res = JSON.stringify(response);
  String res;
  serializeJson(response, res);
  response.clear();
  Serial.println(res);
  return res;
}

void notifyClients(String json_string) {
  ws.textAll(json_string);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, AsyncWebSocketClient *client) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);

    // JSONVar item = JSON.parse((char*)data);
    deserializeJson(item, data);
    if (item["rtype"] == String("get")){
      if (item["rparam"] == String("initial_state")){
        // client->text(getInitialStateString());
        notifyClients(getInitialStateString());
      }
      
    } else 
    if (item["rtype"] == String("set")){
      String value;
      //value = JSON.stringify(item["rvalue"]);
      serializeJson(item["rvalue"], value);
      if (item["rparam"] == String("mode")){
        mode = value.toInt();
        Serial.printf("Mode set to %d. Value is %s. \n", mode, value);

        // client->text(getVariableString("mode"));
        notifyClients(getVariableString("mode"));
      } else
      if (item["rparam"] == String("status")){
        status = value.toInt();
        Serial.printf("Status set to %d. Value is %s.  \n", status, value);
        // client->text(getVariableString("status"));
        notifyClients(getVariableString("status"));
      }
      else
      if (item["rparam"] == String("scene")){
        scene = value.toInt();
        Serial.printf("Scene set to %d. Value is %s.  \n", scene, value);
        // client->text(getVariableString("scene"));
        notifyClients(getVariableString("scene"));
      }
    }
    item.clear();
  }
  
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
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
  // ws.cleanupClients();
}