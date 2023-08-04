#ifndef ACTIVATION_H
#define ACTIVATION_H

#include <Arduino.h>
#include <MFRC522.h>

#define RST_PIN 49
#define SS_PIN 53

#define RED_PIN 26
#define GREEN_PIN 24
#define BLUE_PIN 22

extern bool activationComplete;
extern byte desiredUID[];
extern MFRC522 mfrc522;

void setupActivation();
void runActivation();
bool checkUIDMatch(byte* desiredUID);
void setActivationLED(String colour);

#endif
