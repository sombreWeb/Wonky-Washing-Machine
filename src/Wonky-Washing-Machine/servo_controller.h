#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Servo.h>

#define RELAY_PIN A9
#define TOP_SERVO 7
#define SIDE_SERVO 6
#define BOTTOM_SERVO 5

struct ServoDetails {
  Servo servo;
  int pos = 0;
  int openPos = 180;
  int closePos = 0;
};

const int SWEEP_SPEED = 15;

extern ServoDetails topServo;
extern ServoDetails sideServo;
extern ServoDetails bottomServo;

void servoControllerSetup();

void openDoor(ServoDetails &currentServo);

void closeDoor(ServoDetails &currentServo);

#endif
