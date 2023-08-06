#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Servo.h>

#define RELAY_PIN A9
#define TOP_SERVO 7
#define SIDE_SERVO 6
#define BOTTOM_SERVO 5

const int SWEEP_SPEED = 15;

extern Servo topServo;
extern int topPos;

extern Servo sideServo;
extern int sidePos;

extern Servo bottomServo;
extern int bottomPos;

void servoControllerSetup();

void openDoor(Servo &currentServo, int &currentPos);

void closeDoor(Servo &currentServo, int &currentPos);

#endif
