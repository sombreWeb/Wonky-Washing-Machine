#ifndef WIRES_GAME_H
#define WIRES_GAME_H

#include "wire_calibration.h"
#include "ArduinoSTL.h"

#define SS_PIN_MATRIX 19

struct LevelData {
  std::vector<wireConnection> levelSolution;
  String clue;
};

extern boolean wiresGameComplete;
extern int wiresGameLevel;

void showClue(String clue);
bool checkIfWiresCorrect();
LevelData getLevelData(int level);
std::vector<wireConnection> filterClosestColourValues(std::vector<wireConnection>& connections, float externalAverageResistance);
std::vector<wireConnection> getCurrentConnections();
bool compareByRedPort(const wireConnection& connection1, const wireConnection& connection2);
void flashColours(std::vector<wireConnection> solutionToFlash);
void setupWireGame();
void runWiresGame();

#endif
