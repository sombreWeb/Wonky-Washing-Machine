#include <SPI.h>
#include "servo_controller.h"
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"

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

  servoControllerSetup();

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

  delay(500);
  digitalWrite(SS_PIN_RFID, LOW);
  delay(500);
  runActivation();
  digitalWrite(SS_PIN_RFID, HIGH);

  //runKnobs();

  openDoor(topServo, topPos);

  runPatternGame();

  openDoor(sideServo, sidePos);

  //calibrateRedPorts();
  //calibrateWires();

  delay(500);
  digitalWrite(SS_PIN_MATRIX, LOW);
  delay(500);
  runWiresGame();
  digitalWrite(SS_PIN_MATRIX, HIGH);

  openDoor(bottomServo, bottomPos);

  Serial.print("done");
}

void loop() {}


// Function to print the UID
void printUID(byte* uid, byte uidSize) {
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(uid[i], HEX);
  }
  Serial.println();
}
