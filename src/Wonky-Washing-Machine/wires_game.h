#ifndef WIRE_GAME_H
#define WIRE_GAME_H

#include "wire_calibration.h"
#include "ArduinoSTL.h"

void setupWireGame();
void runWireGame();
std::vector<wireConnection> getCurrentConnections();
std::vector<wireConnection> filterClosestColourValues(std::vector<wireConnection>& connections, float externalAverageResistance);

#endif
