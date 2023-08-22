#include "servo_controller.h"
#include <Arduino.h>
#include <Servo.h>

/**
 * @brief instances of servos and their details for individual control.
 */
ServoDetails topServo;
ServoDetails sideServo;
ServoDetails bottomServo;

/**
   @brief Initializes the servo controller and attached servos.

   This function sets up the servo controller by configuring pin modes, attaching
   servo objects, and initializing servo positions. It also defines close positions
   for the side and bottom servos and closes all doors.
*/
void servoControllerSetup() {
  pinMode(RELAY_PIN, OUTPUT);

  topServo.servo.attach(TOP_SERVO);
  sideServo.servo.attach(SIDE_SERVO);
  bottomServo.servo.attach(BOTTOM_SERVO);

  topServo.pos = topServo.servo.read();
  sideServo.pos = sideServo.servo.read();
  bottomServo.pos = bottomServo.servo.read();

  sideServo.closePos = 50; // Calibrated to match the specific hardware setup.
  bottomServo.closePos = 40; // Calibrated to match the specific hardware setup.

  closeDoor(topServo);
  closeDoor(sideServo);
  closeDoor(bottomServo);
}

/**
   @brief Opens the specified servo-controlled door.

   @param currentServo A reference to the ServoDetails object representing the servo to open.
*/
void openDoor(ServoDetails &currentServo) {
  digitalWrite(RELAY_PIN, HIGH); //on
  delay(500);

   //currentServo.servo.write(currentServo.openPos);
  
  while (currentServo.pos < currentServo.openPos) {
    currentServo.servo.write(currentServo.pos + 1);
    currentServo.pos = currentServo.servo.read();
    delay(SWEEP_SPEED);
  }
  
  delay(500);
  
  digitalWrite(RELAY_PIN, LOW); //off
  delay(500);
}

/**
   @brief Closes the specified servo-controlled door.

   @param currentServo A reference to the ServoDetails object representing the servo to close.
*/
void closeDoor(ServoDetails &currentServo) {
  digitalWrite(RELAY_PIN, HIGH); //on
  delay(500);

  //currentServo.servo.write(currentServo.closePos);
  
  while (currentServo.pos > currentServo.closePos) {
    currentServo.servo.write(currentServo.pos - 1);
    currentServo.pos = currentServo.servo.read();
    delay(SWEEP_SPEED);
  }

  delay(500);

  digitalWrite(RELAY_PIN, LOW); //off
  delay(500);
}
