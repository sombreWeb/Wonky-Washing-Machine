#ifndef KNOBS_H
#define KNOBS_H

#include <Arduino.h>
#include "hub_controller.h"

#define POT_PIN_1 A1
#define POT_PIN_2 A2
#define POT_PIN_3 A3

extern bool knobs_complete;
extern int desired_knob1;
extern int desired_knob2;
extern int desired_knob3;

void setupKnobs();
void runKnobs(HubController &hubController);
int mapPotToNumber(int value);

#endif
