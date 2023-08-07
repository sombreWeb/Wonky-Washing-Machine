#ifndef WIRE_CALIBRATION_H
#define WIRE_CALIBRATION_H

#define PORT_PIN_0 0
#define PORT_PIN_1 1  
#define PORT_PIN_2 2
#define PORT_PIN_3 3

#include "ArduinoSTL.h"

struct wireConnection {
  float resistanceValue;
  int redPort;
  char colour;
  int blackPort;
};

extern const int numberOfResistanceTests, numSensors, sensorPins[];
extern const float redPortOffsets[];
extern float liveConnectionOffsets[];
extern std::vector<wireConnection> allWireConnections;

void calibrationSetup();

bool compareByColourAndBlackPort(const wireConnection &wire1, const wireConnection &wire2);

bool compareByColourAndRedPort(const wireConnection &wire1, const wireConnection &wire2);

bool compareByResistance(const wireConnection &wire1, const wireConnection &wire2);

void updateLiveConnectionOffsets();

void applyLiveConnectionOffsets();

double calculatePercentageDifference(double value1, double value2);

float findAverageResistance(int sensorNumber, int numberOfTests, bool redOffsetOn = true);

std::vector<wireConnection> readCurrentWireSetupCalibration(int redOffset, int blackOffset, bool redOffsetOn = true);

void printWireConnections(std::vector<wireConnection> &values);

void printWireConnectionsRaw(std::vector<wireConnection> &values);

void printCurrentWireConnections();

void calibrateRedPorts();

void calibrateWires();

#endif
