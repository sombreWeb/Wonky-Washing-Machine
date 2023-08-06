#include "servo_controller.h"
#include <Arduino.h> 
#include <Servo.h>

Servo topServo;
int topPos = 0;

Servo sideServo;
int sidePos = 0;

Servo bottomServo;
int bottomPos = 0;

void servoControllerSetup() {
  pinMode(RELAY_PIN, OUTPUT);
  topServo.attach(TOP_SERVO);
  sideServo.attach(SIDE_SERVO);
  bottomServo.attach(BOTTOM_SERVO);
  closeDoor(topServo, topPos);
  closeDoor(sideServo, sidePos);
  closeDoor(bottomServo, bottomPos);
}

void openDoor(Servo &currentServo, int &currentPos) {
  digitalWrite(RELAY_PIN, HIGH); //on
  delay(500);

  for (currentPos = 0; currentPos <= 180; currentPos += 1) {
    currentServo.write(currentPos);
    delay(15);
  }

  digitalWrite(RELAY_PIN, LOW); //off
  delay(500);
}

void closeDoor(Servo &currentServo, int &currentPos) {
  digitalWrite(RELAY_PIN, HIGH); //on
  delay(500);

  for (currentPos = 180; currentPos >= 0; currentPos -= 1)  {
    currentServo.write(currentPos);
    delay(15);
  }

  digitalWrite(RELAY_PIN, LOW); //off
  delay(500);
}
