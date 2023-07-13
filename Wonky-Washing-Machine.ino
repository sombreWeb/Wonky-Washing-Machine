#include <SPI.h>

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A7));
 
  SPI.begin();
  
  setupActivation();
  setupKnobs();
  setupPatternGame();
  
  runActivation();
  runKnobs();
  runPatternGame();

  Serial.print("done");

}

void loop() {}


// Function to print the UID
void printUID(byte* uid, byte uidSize) {
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] < 0x10) {      // If the byte value is less than 0x10 (single-digit hexadecimal), add a leading zero
      Serial.print("0");
    }
    Serial.print(uid[i], HEX);  // Print the byte value as a hexadecimal value
  }
  Serial.println();            // Move to a new line after printing the UID
}
