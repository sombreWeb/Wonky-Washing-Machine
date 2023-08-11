
#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define RXD2 16
#define TXD2 17

const char* ssid = "VM9449079"; //Enter SSID
const char* password = "jbmhcvLtn4wr"; //Enter Password
const char* websockets_server_host = "192.168.0.22"; //Enter server adress
const uint16_t websockets_server_port = 8000; // Enter server port

using namespace websockets;
WebsocketsClient client;

boolean wifiConnected = false;
boolean serverHubConnected = false;
boolean serverHubRegistered = false;

String serverMessageBuffer = "";

void setup() {

  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, password);

  Serial.println("\nConnecting to wifi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  wifiConnected = true;
  Serial.println("\nConnected to Wifi \nConnecting to server hub");

  bool connected = false;

  while (!connected) {
    connected = client.connect(websockets_server_host, websockets_server_port, "/connect");
    Serial.print(".");
    delay(1000);
  }

  serverHubConnected = true;
  Serial.println("\nConnected to server hub");

  client.onMessage(onMessageCallback);
}

void onMessageCallback(WebsocketsMessage message) {
  String serverMessage = message.data();
  Serial.print("Got Message from server hub: ");
  Serial.println(serverMessage);
  Serial2.println(serverMessage);
  serverMessageBuffer = serverMessage;
}

void loop() {
  // let the websockets client check for incoming messages
  if (client.available()) {
    client.poll();
  }

  if (Serial2.available()) {
    String receivedCommand = Serial2.readStringUntil('\n');

    receivedCommand.trim();

    Serial.print("Command received:");
    Serial.println(receivedCommand);


    // Handle wifi status request
    if (receivedCommand == "wifiStatus") {
      if (wifiConnected) {
        Serial2.println("{\"wifiConnected\":\"wifi-connected\"}");
        Serial.println("{\"wifiConnected\":\"wifi-connected\"}");
        serverMessageBuffer = "{\"wifiConnected\":\"wifi-connected\"}";
      }
      return;
    }

    // Handle serverHub status request
    if (receivedCommand == "serverHubStatus") {
      if (serverHubConnected) {
        Serial2.println("{\"serverHubConnected\":\"serverHub-connected\"}");
        serverMessageBuffer = "{\"serverHubConnected\":\"serverHub-connected\"}";
      }
      return;
    }

    // Handle resend server message request
    if (receivedCommand == "resend") {
      Serial.print("Resending message: ");
      Serial.println(serverMessageBuffer);
      Serial2.println(serverMessageBuffer);
      return;
    }

    // Json commands
    StaticJsonDocument<2000> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, receivedCommand);

    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Register handler
    if (jsonDoc.containsKey("action") && jsonDoc["action"].as<String>() == "register") {
      Serial.println("Received 'register' command");
      client.send(receivedCommand);
      serverHubRegistered = true;
    }

    // Update handler
    if (jsonDoc.containsKey("action") && jsonDoc["action"].as<String>() == "update") {
      Serial.println("Received 'update' command");
      client.send(receivedCommand);
    }
  }

  delay(500);
}
