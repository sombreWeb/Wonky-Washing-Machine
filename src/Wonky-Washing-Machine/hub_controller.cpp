#include <ArduinoJson.h>
#include <SPI.h>
#include "servo_controller.h"
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"
#include "hub_controller.h"


/*
   NOTE" On the Arduino mega you need to manually change the internal RX and TX buffer sizes.
*/

String registerRoomId = "1";

boolean wifiConnected = false;
boolean serverHubConnected = false;
boolean serverHubRegistered = false;

int invalidBufferRetry = 50;

void HubController::setupHub() {

  Serial.println("Setting up wifi...");
  while (!wifiConnected) {
    Serial2.println("wifiStatus");
    delay(2000);
    checkHub();
  }
  Serial.println("Wifi connected!");

  Serial.println("Setting up server hub...");
  while (!serverHubConnected) {
    Serial2.println("serverHubStatus");
    delay(2000);
    checkHub();
  }
  Serial.println("Server hub connected!");

  Serial.println("Registering with server hub...");
  while (!serverHubRegistered) {
    String registerStr = getRegisterStr(registerRoomId);
    Serial.print("Register string sent: ");
    Serial.println(registerStr);
    Serial2.println(registerStr);
    delay(10000);
    checkHub();
  }
  Serial.println("Registered with server hub!");
}

void HubController::processMessage(String jsonString) {

  StaticJsonDocument<2000> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  if (error){
    Serial.print("JSON Deserialization Error: ");
    Serial.println(error.c_str());
    return;
  }

  String wifiConnectedJson = jsonDoc["wifiConnected"];
  String serverHubConnectedJson = jsonDoc["serverHubConnected"];

  if (wifiConnectedJson == "wifi-connected") {
    wifiConnected = "true";
    return;
  }

  if (serverHubConnectedJson == "serverHub-connected") {
    serverHubConnected = "true";
    return;
  }

  String errorJson = jsonDoc["error"];
  String messageJson = jsonDoc["message"];
  String typeJson = jsonDoc["type"];
  String deviceIdJson = jsonDoc["deviceid"];

  if (errorJson == "false" && messageJson == "" && typeJson == "" && deviceIdJson.length() > 0) {
    serverHubRegistered = true;
    return;
  }

  String actionJson = jsonDoc["action"];
  String actionidJson = jsonDoc["actionid"];
  String deviceidJson = jsonDoc["deviceid"];
  String roomidJson = jsonDoc["roomid"];

  if (actionJson == "action") {
    if (actionidJson == "openAllDoors") {
      openDoor(topServo);
      openDoor(sideServo);
      openDoor(bottomServo);
    }

    if (actionidJson == "closeAllDoors") {
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
      activationComplete = false;
      knobs_complete = false;
      patternGameComplete = false;
      wiresGameComplete = false;

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

void HubController::addAction(JsonArray & actions, String actionId, String actionName, boolean enabled) {
  JsonObject actionObj = actions.createNestedObject();
  actionObj["actionid"] = actionId;
  actionObj["name"] = actionName;
  actionObj["enabled"] = enabled;
}

String HubController::getRegisterStr(String room) {
  StaticJsonDocument<2000> jsonDoc;
  jsonDoc["action"] = "register";
  jsonDoc["room"] = room;
  jsonDoc["name"] = "Wonky Washing Machine";
  jsonDoc["status"] = "Ready to Start";

  JsonArray actionsArray = jsonDoc.createNestedArray("actions");

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

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  return jsonString;
}

bool HubController::isValidJSON(String jsonString) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  return error == DeserializationError::Ok;
}

void HubController::checkHub() {

  if (Serial2.available()) {

    for (int attemptIdx = 0; attemptIdx < invalidBufferRetry; attemptIdx++) {

      String command = Serial2.readStringUntil('\n');
      command.trim();

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
