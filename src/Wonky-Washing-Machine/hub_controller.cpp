#include <ArduinoJson.h>
#include <SPI.h>
#include "servo_controller.h"
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"

void processMessage(String incomingMessage) {

  StaticJsonDocument<200> doc;

  DeserializationError error = deserializeJson(doc, incomingMessage);

  if (error) {
    Serial.print("Parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  String action = doc["action"];
  String actionid = doc["actionid"];
  String deviceid = doc["deviceid"];
  String roomid = doc["roomid"];

  if (action == "action") {

    if (actionid == "openAllDoors") {
      openDoor(topServo);
      openDoor(sideServo);
      openDoor(bottomServo);
    }

    if (actionid == "closeAllDoors") {
      closeDoor(topServo);
      closeDoor(sideServo);
      closeDoor(bottomServo);
    }

    if (actionid == "completeAll") {
      activationComplete = true;
      knobs_complete = true;
      patternGameComplete = true;
      wiresGameComplete = true;
    }

    if (actionid == "completeActivation") {
      activationComplete = true;
    }

    if (actionid == "completeKnobsGame") {
      knobs_complete = true;
    }

    if (actionid == "completePatternGame") {
      patternGameComplete = true;
    }

    if (actionid == "completeWireGame") {
      wiresGameComplete = true;
    }

    if (actionid == "resetPuzzle") {
      activationComplete = false;
      knobs_complete = false;
      patternGameComplete = false;
      wiresGameComplete = false;

      closeDoor(topServo);
      closeDoor(sideServo);
      closeDoor(bottomServo);

      //runPuzzle();
    }

    if (actionid == "setPatternGameLevel1") {
      patternGameLevel = 1;
    }

    if (actionid == "setPatternGameLevel2") {
      patternGameLevel = 2;
    }

    if (actionid == "setPatternGameLevel3") {
      patternGameLevel = 3;
    }

    if (actionid == "setWiresGameLevel1") {
      wiresGameLevel = 1;
    }

    if (actionid == "setWiresGameLevel2") {
      wiresGameLevel = 2;
    }

    if (actionid == "setWiresGameLevel3") {
      wiresGameLevel = 3;
    }

  }
}


void addAction(JsonArray &actions, String actionId, String actionName, boolean enabled) {
  JsonObject actionObj = actions.createNestedObject();
  actionObj["actionid"] = actionId;
  actionObj["name"] = actionName;
  actionObj["enabled"] = enabled;
}

String getRegisterStr(String room) {
  StaticJsonDocument<512> jsonDoc;
  jsonDoc["action"] = "register";
  jsonDoc["room"] = room;
  jsonDoc["name"] = "Wonky Washing Machine";
  jsonDoc["status"] = "Ready to Start";

  JsonArray actionsArray = jsonDoc.createNestedArray("actions");

  addAction(actionsArray, "openAllDoors", "Open All Doors", false);
  addAction(actionsArray, "closeAllDoors", "Close All Doors", false);
  addAction(actionsArray, "completeAll", "Complete All", false);
  addAction(actionsArray, "completeActivation", "Complete Activation", false);
  addAction(actionsArray, "completeKnobsGame", "Complete Knobs Game", false);
  addAction(actionsArray, "completePatternGame", "Complete Pattern Game", false);
  addAction(actionsArray, "completeWireGame", "Complete Wire Game", false);
  addAction(actionsArray, "resetPuzzle", "Reset Puzzle", false);
  addAction(actionsArray, "setPatternGameLevel1", "Set Pattern Game Level 1", false);
  addAction(actionsArray, "setPatternGameLevel2", "Set Pattern Game Level 2", false);
  addAction(actionsArray, "setPatternGameLevel3", "Set Pattern Game Level 3", false);
  addAction(actionsArray, "setWiresGameLevel1", "Set Wires Game Level 1", false);
  addAction(actionsArray, "setWiresGameLevel2", "Set Wires Game Level 2", false);
  addAction(actionsArray, "setWiresGameLevel3", "Set Wires Game Level 3", false);

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  return jsonString;
}
