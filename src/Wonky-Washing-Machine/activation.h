#ifndef ACTIVATION_H
#define ACTIVATION_H

#include <Arduino.h>
#include <MFRC522.h>
#include "hub_controller.h"

#define RST_PIN_RFID 49
#define SS_PIN_RFID 53

#define RED_PIN 48
#define GREEN_PIN 46
#define BLUE_PIN 47

extern bool activationComplete;
extern byte desiredUID[];
extern MFRC522 mfrc522;

void setupActivation();
void runActivation(HubController &hubController);
bool checkUIDMatch(byte* desiredUID);
void setActivationLED(String colour);
void printUID(byte* uid, byte uidSize);

#endif
