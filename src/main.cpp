#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <credentials.h>
#include <variables.h>
#include <ArduinoJson.h>
const char *ssid = BOCIASAN_WIFI_SSID;
const char *password = BOCIASAN_WIFI_PASS;

// Webserver
AsyncWebServer server(80);
//Websocket
AsyncWebSocket ws("/ws");

String state;

const int LED_PIN = LED_BUILTIN;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

String getInitialStateString(){
  char res[512];
  char weekly_stats_string[128];
  sprintf(weekly_stats_string, "[[%d,%d],[%d,%d],[%d,%d],[%d,%d]]", weekly_stats[0][0], weekly_stats[0][1], weekly_stats[1][0], weekly_stats[1][1],weekly_stats[2][0], weekly_stats[2][1], weekly_stats[3][0],weekly_stats[3][1]);
  sprintf(res, "{\"mode\":%d,\"status\":%d,\"target\":%f,\"current_temp\":%f,\"scene\":%d,\"esp_time\":%u,\"ssid\":\"%s\",\"rssi\":%d,\"city\":\"%s\",\"rtype\":\"get\",\"rparam\":\"initial_state\",\"weekly_stats\":%s}", 
    mode, status, (round(target * 100) / 100.0), (round(current_temp * 100) / 100.0), scene, esp_time, WiFi.SSID(), WiFi.RSSI(),city,weekly_stats_string);
  return (String)res;
}

String getVariableString(String rparam){
  char res[128];
  int rvalue = 0;
  String rtype = "get";

  if (rparam == "mode"){
    rvalue = mode;
  } else 
  if (rparam == "status"){
    rvalue = status;
  }else 
  if (rparam == "scene"){
    rvalue = scene;
  }
  sprintf(res, "{\"rtype\":\"%s\", \"rparam\":\"%s\", \"rvalue\":%d}", rtype, rparam, rvalue);
  Serial.println(res);

  return (String)res;
}

void notifyClients(String json_string) {
  ws.textAll(json_string);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, AsyncWebSocketClient *client) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);
    StaticJsonDocument<200> item;
    esp_time = millis();
    // JSONVar item = JSON.parse((char*)data);
    deserializeJson(item, data);
    if (item["rtype"] == String("get")){
      if (item["rparam"] == String("initial_state")){
        client->text(getInitialStateString());
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
  ws.cleanupClients();
}