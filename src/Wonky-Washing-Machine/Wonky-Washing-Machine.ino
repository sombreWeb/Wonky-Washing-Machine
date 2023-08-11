#include <SPI.h>
#include "servo_controller.h"
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"
#include "hub_controller.h"

HubController hubController;

void setup() {

  Serial.println("Setting up puzzle...");

  Serial.begin(9600);
  Serial2.begin(9600);

  randomSeed(analogRead(A0));

  SPI.begin();

  hubController.setupHub();

  // Set SS pins as OUTPUT and HIGH (off)
  pinMode(SS_PIN_RFID, OUTPUT);
  digitalWrite(SS_PIN_RFID, HIGH);
  pinMode(SS_PIN_MATRIX, OUTPUT);
  digitalWrite(SS_PIN_MATRIX, HIGH);

  setupKnobs();

  setupPatternGame();

  delay(500);
  digitalWrite(SS_PIN_MATRIX, LOW);
  delay(500);
  setupWireGame();
  digitalWrite(SS_PIN_MATRIX, HIGH);

  delay(500);
  digitalWrite(SS_PIN_RFID, LOW);
  delay(500);
  setupActivation();
  digitalWrite(SS_PIN_RFID, HIGH);

  //while (checkButtonPressed() < 0);
  //delay(10000);

  servoControllerSetup();

  Serial.println("----- Puzzle setup complete -----");

  //runPuzzle();
}

void loop() {
  hubController.checkHub();
  }

void runPuzzle() {

  Serial.println("Running puzzle...");
  hubController.checkHub();

  delay(500);
  digitalWrite(SS_PIN_RFID, LOW);
  delay(500);
  runActivation();
  digitalWrite(SS_PIN_RFID, HIGH);
  hubController.checkHub();

  runKnobs();
  hubController.checkHub();

  openDoor(topServo);
  hubController.checkHub();

  runPatternGame();
  hubController.checkHub();

  openDoor(sideServo);
  hubController.checkHub();

  //calibrateRedPorts();
  //calibrateWires();

  delay(500);
  digitalWrite(SS_PIN_MATRIX, LOW);
  delay(500);
  runWiresGame();
  hubController.checkHub();
  digitalWrite(SS_PIN_MATRIX, HIGH);

  openDoor(bottomServo);
  hubController.checkHub();

  Serial.print("Puzzle complete!");
}

void checkForReset() {

  if (hubController.hubPuzzleResetRequest) {
    hubController.hubPuzzleResetRequest = false;
    runPuzzle();
  }
}
