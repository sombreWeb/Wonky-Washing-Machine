#include <SPI.h>
#include "servo_controller.h"
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"
#include "hub_controller.h"

String mss = "{\"action\": \"action\","
             "\"actionid\": \"setPatternGameLevel1\","
             "\"deviceid\": \"cf29a - 44194 - abcdef\","
             "\"roomid\": \"1\"}";

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(A0));

  SPI.begin();

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

  Serial.println(getRegisterStr("1"));
  
  //processMessage(mss);

  runPuzzle();
}

void loop() {}

void runPuzzle() {

  Serial.println("running...");

  delay(500);
  digitalWrite(SS_PIN_RFID, LOW);
  delay(500);
  runActivation();
  digitalWrite(SS_PIN_RFID, HIGH);

  runKnobs();

  openDoor(topServo);

  runPatternGame();

  openDoor(sideServo);

  //calibrateRedPorts();
  //calibrateWires();

  delay(500);
  digitalWrite(SS_PIN_MATRIX, LOW);
  delay(500);
  runWiresGame();
  digitalWrite(SS_PIN_MATRIX, HIGH);

  openDoor(bottomServo);

  Serial.print("done");
}
