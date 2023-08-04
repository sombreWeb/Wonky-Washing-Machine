#include <SPI.h>
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"

void setup() {
  Serial.begin(9600);
  SPI.begin();
  
  randomSeed(analogRead(A0));

  //calibrateRedPorts();
  //calibrateWires();
 
  //setupActivation();
  //setupKnobs();
  //setupPatternGame();
  setupWireGame();
  
  //runActivation();
  //runKnobs();
  //runPatternGame();
  runWiresGame();

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
