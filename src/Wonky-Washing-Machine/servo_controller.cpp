#include "servo_controller.h"
#include <Arduino.h>
#include <Servo.h>

ServoDetails topServo;

ServoDetails sideServo;

ServoDetails bottomServo;


void servoControllerSetup() {
  pinMode(RELAY_PIN, OUTPUT);

  topServo.servo.attach(TOP_SERVO);
  sideServo.servo.attach(SIDE_SERVO);
  bottomServo.servo.attach(BOTTOM_SERVO);

  topServo.pos = topServo.servo.read();
  sideServo.pos = sideServo.servo.read();
  bottomServo.pos = bottomServo.servo.read();

  delay(5000);
  closeDoor(topServo);
    delay(5000);
  closeDoor(sideServo);
    delay(5000);
  closeDoor(bottomServo);
}

void openDoor(ServoDetails &currentServo) {
  digitalWrite(RELAY_PIN, HIGH); //on
  delay(500);

  while (currentServo.pos < currentServo.openPos) {
    currentServo.servo.write(currentServo.pos + 1);
    currentServo.pos = currentServo.servo.read();
    delay(SWEEP_SPEED);
  }

  digitalWrite(RELAY_PIN, LOW); //off
  delay(500);
}

void closeDoor(ServoDetails &currentServo) {
  digitalWrite(RELAY_PIN, HIGH); //on
  delay(500);

  

  while (currentServo.pos > currentServo.closePos) {
    currentServo.servo.write(currentServo.pos - 1);
    currentServo.pos = currentServo.servo.read();
    delay(SWEEP_SPEED);
  }
  

  digitalWrite(RELAY_PIN, LOW); //off
  delay(500);
}
