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
    if (uid[i] < 0x10) {      
      Serial.print("0");
    }
    Serial.print(uid[i], HEX);  
  }
  Serial.println(); 
}
