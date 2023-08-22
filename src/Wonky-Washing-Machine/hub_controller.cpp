#include <ArduinoJson.h>
#include <SPI.h>
#include "servo_controller.h"
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"
#include "hub_controller.h"

// NOTE: On the Arduino Mega, you may need to manually change the internal RX and TX buffer sizes (128).

/**
   @brief Room ID for registration
*/
String registerRoomId = "1";

/**
   @brief Flags for tracking connections and registration status
*/
boolean wifiConnected = false;
boolean serverHubConnected = false;
boolean serverHubRegistered = false;

/**
   @brief Time intervals for checking connection and registration status
*/
int timeBetweenWifiConnectionCheck = 2000;
int timeBetweenServerHubConnectionCheck = 2000;
int timeBetweenRegisterCheck = 10000;

/**
   @brief Retry count for handling invalid buffers
*/
int invalidBufferRetry = 50;

/**
   @brief JSON string for registration data
*/
String registrationJson;

/**
   @brief Sets up the hub controller, including Wi-Fi, server hub, and registration.
*/
void HubController::setupHub() {

  if (!hubEnabled) {
    return;
  }

  Serial.println("Setting up wifi...");
  while (!wifiConnected) {
    Serial2.println("wifiStatus");
    delay(timeBetweenWifiConnectionCheck);
    checkHub();
  }
  Serial.println("Wifi connected!");

  Serial.println("Setting up server hub...");
  while (!serverHubConnected) {
    Serial2.println("serverHubStatus");
    delay(timeBetweenServerHubConnectionCheck);
    checkHub();
  }
  Serial.println("Server hub connected!");

  Serial.println("Registering with server hub...");
  while (!serverHubRegistered) {
    generateRegistrationJson(registerRoomId, "Ready to start");
    Serial.print("Register string sent: ");
    Serial.println(registrationJson);
    Serial2.println(registrationJson);
    delay(timeBetweenRegisterCheck);
    checkHub();
  }
  Serial.println("Registered with server hub!");

  // Change to update
  StaticJsonDocument<1500> doc;
  DeserializationError error = deserializeJson(doc, registrationJson);
  if (error) {
    Serial.print("Error parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }
  doc["action"] = "update";
  serializeJson(doc, registrationJson);
}

/**
   @brief Processes an incoming JSON message from the server.

   @param jsonString The JSON message to process.
*/
void HubController::processMessage(String jsonString) {

  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print("JSON Deserialization Error: ");
    Serial.println(error.c_str());
    return;
  }

  String wifiConnectedJson = doc["wifiConnected"];
  String serverHubConnectedJson = doc["serverHubConnected"];

  if (wifiConnectedJson == "wifi-connected") {
    wifiConnected = "true";
    return;
  }

  if (serverHubConnectedJson == "serverHub-connected") {
    serverHubConnected = "true";
    return;
  }

  String errorJson = doc["error"];
  String messageJson = doc["message"];
  String typeJson = doc["type"];
  String deviceIdJson = doc["deviceid"];

  if (errorJson == "false" && messageJson == "" && typeJson == "" && deviceIdJson.length() > 0) {
    serverHubRegistered = true;
    return;
  }

  String actionJson = doc["action"];
  String actionidJson = doc["actionid"];
  String deviceidJson = doc["deviceid"];
  String roomidJson = doc["roomid"];

  if (actionJson == "action") {

    if (actionidJson == "openAllDoors") {
      Serial.println("Doors open command");
      openDoor(topServo);
      openDoor(sideServo);
      openDoor(bottomServo);
    }

    if (actionidJson == "closeAllDoors") {
      Serial.println("Doors close command");
      closeDoor(topServo);
      closeDoor(sideServo);
      closeDoor(bottomServo);
    }

    if (actionidJson == "completeAll") {
      Serial.println("All complete");
      activationComplete = true;
      knobs_complete = true;
      patternGameComplete = true;
      wiresGameComplete = true;
    }

    if (actionidJson == "completeActivation") {
      activationComplete = true;
    }

    if (actionidJson == "completeKnobsGame") {
      knobs_complete = true;
    }

    if (actionidJson == "completePatternGame") {
      patternGameComplete = true;
    }

    if (actionidJson == "completeWireGame") {
      wiresGameComplete = true;
    }

    if (actionidJson == "resetPuzzle") {
      activationComplete = true;
      knobs_complete = true;
      patternGameComplete = true;
      wiresGameComplete = true;

      fillLights("black");
      showClue("        ");

      closeDoor(topServo);
      closeDoor(sideServo);
      closeDoor(bottomServo);

      hubPuzzleResetRequest = true;
    }

    if (actionidJson == "setPatternGameLevel1") {
      patternGameLevel = 1;
    }

    if (actionidJson == "setPatternGameLevel2") {
      patternGameLevel = 2;
    }

    if (actionidJson == "setPatternGameLevel3") {
      patternGameLevel = 3;
    }

    if (actionidJson == "setWiresGameLevel1") {
      wiresGameLevel = 1;
    }

    if (actionidJson == "setWiresGameLevel2") {
      wiresGameLevel = 2;
    }

    if (actionidJson == "setWiresGameLevel3") {
      wiresGameLevel = 3;
    }
  }
}

/**
   @brief Adds an action to the JSON registration data.

   @param actions The JSON array where the action should be added.
   @param actionId The ID of the action.
   @param actionName The name of the action.
   @param enabled Whether the action is enabled.
*/
void HubController::addAction(JsonArray & actions, String actionId, String actionName, boolean enabled) {
  JsonObject actionObj = actions.createNestedObject();
  actionObj["actionid"] = actionId;
  actionObj["name"] = actionName;
  actionObj["enabled"] = enabled;
}

/**
   @brief Generates JSON registration data based on room number and a start status.

   @param room The room ID for registration.
   @param statusStr The status string to include in the registration data.
*/
void HubController::generateRegistrationJson(String room, String statusStr) {

  registrationJson = "";

  StaticJsonDocument<1500> doc;

  doc["action"] = "register";
  doc["room"] = room;
  doc["name"] = "Wonky Washing Machine";
  doc["status"] = statusStr;

  JsonArray actionsArray = doc.createNestedArray("actions");

  addAction(actionsArray, "openAllDoors", "Open All Doors", true);
  addAction(actionsArray, "closeAllDoors", "Close All Doors", true);
  addAction(actionsArray, "completeAll", "Complete All", true);
  addAction(actionsArray, "completeActivation", "Complete Activation", true);
  addAction(actionsArray, "completeKnobsGame", "Complete Knobs Game", true);
  addAction(actionsArray, "completePatternGame", "Complete Pattern Game", true);
  addAction(actionsArray, "completeWireGame", "Complete Wire Game", true);
  addAction(actionsArray, "resetPuzzle", "Reset Puzzle", true);
  addAction(actionsArray, "setPatternGameLevel1", "Set Pattern Game Level 1", true);
  addAction(actionsArray, "setPatternGameLevel2", "Set Pattern Game Level 2", true);
  addAction(actionsArray, "setPatternGameLevel3", "Set Pattern Game Level 3", true);
  addAction(actionsArray, "setWiresGameLevel1", "Set Wires Game Level 1", true);
  addAction(actionsArray, "setWiresGameLevel2", "Set Wires Game Level 2", true);
  addAction(actionsArray, "setWiresGameLevel3", "Set Wires Game Level 3", true);

  serializeJson(doc, registrationJson);
}

/**
   @brief Updates the hub's status with new registration data.

   @param room The room ID for registration.
   @param statusStr The status string to include in the registration data.
*/
void HubController::updateStatus(String room, String statusStr) {
  generateRegistrationJson(room, statusStr);
  Serial2.println(registrationJson);
}

/**
   @brief Checks if a given JSON string is valid.

   @param jsonString The JSON string to validate.

   @return true if the JSON string is valid, false otherwise.
*/
bool HubController::isValidJSON(String jsonString) {
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  return error == DeserializationError::Ok;
}

/**
   @brief Checks the hub for incoming commands and processes them.
*/
void HubController::checkHub() {

  if (!hubEnabled) {
    return;
  }

  if (Serial2.available()) {

    for (int attemptIdx = 0; attemptIdx < invalidBufferRetry; attemptIdx++) {

      Serial.println("Command incomming");


      String command = Serial2.readString();

      Serial.println("Command to string error");

      command.trim();

      if (command == "") {
        return;
      }

      Serial.print("Raw command receieved: ");
      Serial.println(command);

      boolean isValidJsonString = isValidJSON(command);

      if (isValidJsonString) {
        processMessage(command);
        Serial.print("REAL Command received --- ");
        Serial.println(command);
        break;
      } else {
        Serial.print("Proto Command received --- ");
        Serial.println(command);
        Serial2.println("resend");
        continue;
      }
    }
  }
}
