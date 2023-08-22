
#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define RXD2 16
#define TXD2 17

const char* ssid = "EscapeRoom"; // Replace with hub ssid
const char* password = "LetMeOut!"; // Replace with hub password
const char* websockets_server_host = "192.168.0.2"; // IP address of the WebSocket server. Replace with server's IP address
const uint16_t websockets_server_port = 8000; // Port of the WebSocket server. Replace with server's port

using namespace websockets;
WebsocketsClient client;

/**
  @brief Flag indicating WiFi connection status.
*/
boolean wifiConnected = false;

/**
  @brief Flag indicating WebSocket server connection status.
*/
boolean serverHubConnected = false;

/**
   @brief Flag indicating registration with the server ststus.
*/
boolean serverHubRegistered = false;

/**
   @brief Buffer to store latest server message.
*/
String serverMessageBuffer = "";

/**
   @brief Setup function for the esp-32.

   - Connect to wifi.
   - Establish WebSocket connection with server.
   - Setup callback for incomming messages from server hub.
*/
void setup() {

  // Begin serial communication.
  Serial.begin(9600); // Serial used for local monitoring.
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // Serial2 used for sending messages to arduio.

  // Wifi connection
  WiFi.begin(ssid, password);

  Serial.println("\nConnecting to wifi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  wifiConnected = true;

  // Establish WebSocket connection with server.
  Serial.println("\nConnected to Wifi \nConnecting to server hub");

  bool connected = false;
  while (!connected) {
    connected = client.connect(websockets_server_host, websockets_server_port, "/connect");
    Serial.print(".");
    delay(1000);
  }
  serverHubConnected = true;
  Serial.println("\nConnected to server hub");

  // Setup callback for incomming messages from server hub.
  client.onMessage(onMessageCallback);
}

/**
   @brief Callback function for handling WebSocket messages.
   @param message The WebSocket message received from the server.
*/
void onMessageCallback(WebsocketsMessage message) {
  String serverMessage = message.data();
  Serial.print("Got Message from server hub: ");
  Serial.println(serverMessage); // Display message locally.
  Serial2.println(serverMessage); // Relay message to arduino.
  serverMessageBuffer = serverMessage; // Load latest message into buffer.
}

/**
   @brief Loop function for the esp-32.

   - Check for server hub messages.
   - Check for commands from arduino.
   - Handle incomming arduino commands.
*/
void loop() {
  // WebSocket client check for incomming messages from server
  if (client.available()) {
    client.poll();
  }

  // Check for commands from arduino
  if (Serial2.available()) {

    // Get the command.
    String receivedCommand = Serial2.readStringUntil('\n');
    receivedCommand.trim();

    // Print command locally.
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
