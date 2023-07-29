#ifndef WIRES_GAME_H
#define WIRES_GAME_H

#include "wire_calibration.h"
#include "ArduinoSTL.h"

void setupWireGame();
void runWiresGame();
bool checkIfWiresCorrect();
std::vector<wireConnection> getSolution(int level);
std::vector<wireConnection> filterClosestColourValues(std::vector<wireConnection>& connections, float externalAverageResistance);
std::vector<wireConnection> getCurrentConnections();
bool compareByRedPort(const wireConnection& connection1, const wireConnection& connection2);

#endif
