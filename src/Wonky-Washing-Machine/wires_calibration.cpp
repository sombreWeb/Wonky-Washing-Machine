#include "ArduinoSTL.h"
#include "wire_calibration.h"

const int numSensors = 4;
const int sensorPins[] = { A1, A2, A3, A4 };
int sensorValues[numSensors] = { 0 };
float Vouts[numSensors] = { 0 };
float Vin = 5;
float Rref = 20000;

int homePort = 0;
const float redPortOffsets[numSensors] = {0.00, -0.0234, -0.0179, -0.0147};

const int numberOfResistanceTests = 50;

std::vector<wireConnection> allWireConnections =
{
  {4503.76f, 0, 'r', 0},
  {7830.79f, 0, 'r', 1},
  {9483.86f, 0, 'r', 2},
  {12947.08f, 0, 'r', 3},
  {17951.27f, 1, 'g', 0},
  {20878.36f, 1, 'g', 1},
  {22810.22f, 1, 'g', 2},
  {26135.48f, 1, 'g', 3},
  {33745.24f, 2, 'b', 0},
  {37460.39f, 2, 'b', 1},
  {38621.08f, 2, 'b', 2},
  {42251.26f, 2, 'b', 3},
  {52172.80f, 3, 'y', 0},
  {55292.11f, 3, 'y', 1},
  {57618.33f, 3, 'y', 2},
  {60319.80f, 3, 'y', 3},
};

bool compareByColourAndBlackPort(const wireConnection &wire1, const wireConnection &wire2) {
  if (wire1.colour != wire2.colour) {
    return wire1.colour < wire2.colour;
  } else {
    return wire1.blackPort < wire2.blackPort;
  }
}

bool compareByColourAndRedPort(const wireConnection &wire1, const wireConnection &wire2) {
  if (wire1.colour != wire2.colour) {
    return wire1.colour < wire2.colour;
  } else {
    return wire1.redPort < wire2.redPort;
  }
}

bool compareByResistance(const wireConnection &wire1, const wireConnection &wire2) {
  return wire1.resistanceValue < wire2.resistanceValue;
}

double calculatePercentageDifference(double value1, double value2) {
  double percentageDiff = ((value1 - value2) / ((value1 + value2) / 2.0)) * 100.0;
  if (isnan(percentageDiff)) {
    return 0.0;
  }
  return percentageDiff;
}

float findAverageResistance(int sensorNumber, int numberOfTests, boolean redOffsetOn = true) {
  float redOffset = Rref * (1 + redPortOffsets[sensorNumber]);

  float sumResistance = 0;
  for (int j = 0; j < numberOfTests; j++) {
    sensorValues[sensorNumber] = analogRead(sensorPins[sensorNumber]);
    Vouts[sensorNumber] = (Vin * sensorValues[sensorNumber]) / 1023.0;
    float resistance = Rref * (1.0 / ((Vin / Vouts[sensorNumber]) - 1.0));
    sumResistance += resistance;
    delay(5);  // Delay to allow the ADC to stabilize between readings
  }
  float averageResistance = sumResistance / numberOfTests;  // Calculate average resistance
  if (redOffsetOn) {
    averageResistance = averageResistance * (1 + redPortOffsets[sensorNumber]);
  }
  return averageResistance;
}

std::vector<wireConnection> readCurrentWireSetupCalibration(int redOffset, int blackOffset, boolean redOffsetOn = true) {
  char colours[] = { 'r', 'g', 'b', 'y' };
  std::vector<wireConnection> currentWireSetup;
  for (int sensorNum = 0; sensorNum < numSensors; sensorNum++) {
    float averageRestistance = findAverageResistance(sensorNum, numberOfResistanceTests, redOffsetOn);
    int colorIndex = (sensorNum - redOffset + numSensors) % numSensors;
    currentWireSetup.push_back({ averageRestistance, sensorNum, colours[colorIndex], ((colorIndex + blackOffset) % numSensors) });
  }
  return currentWireSetup;
}

void printWireConnections(std::vector<wireConnection> &values) {

  for (const auto &connection : values) {
    Serial.print("Resistance: ");
    Serial.print(connection.resistanceValue);
    Serial.print(" Ohms, Red Port: ");
    Serial.print(connection.redPort);
    Serial.print(", Colour: ");
    Serial.print(String(connection.colour));
    Serial.print(", Black Port: ");
    Serial.println(connection.blackPort);
  }
  Serial.println("---------------");
}

void printWireConnectionsRaw(std::vector<wireConnection> &values) {

  Serial.println("{");
  for (const auto &connection : values) {
    Serial.print("    {");
    Serial.print(connection.resistanceValue, 2); // 2 decimal places
    Serial.print("f, ");
    Serial.print(connection.redPort);
    Serial.print(", '");
    Serial.print(connection.colour);
    Serial.print("', ");
    Serial.print(connection.blackPort);
    Serial.println("},");
  }
  Serial.println("};");
}

void printCurrentWireConnections() {
  std::vector<wireConnection> currentValues = readCurrentWireSetupCalibration(0, 0);
  std::sort(currentValues.begin(), currentValues.end(), compareByColourAndBlackPort);
  printWireConnections(currentValues);
}

void calibrateRedPorts() {
  std::vector<wireConnection> calibrationValues;

  boolean firstRedCalibration = true;
  for (int moveRedWires = 0; moveRedWires < numSensors; moveRedWires++) {
    do {
      if (firstRedCalibration) {
        Serial.println("Setup the red connections in the natural position RGBY from left to right. Enter ok to continue.");
        firstRedCalibration = false;
      } else {
        Serial.println("Move each red connection one to the right. Enter ok to continue.");
      }

      while (Serial.available() == 0) {
      }

      String userInput = Serial.readStringUntil('\n');

      if (userInput.equalsIgnoreCase("ok")) {
        Serial.println("Accepted.");
        break;
      } else {
        Serial.println("Invalid input. Please enter 'ok'.");
      }
    } while (true);
    std::vector<wireConnection> currentValues = readCurrentWireSetupCalibration(moveRedWires, 0, false);
    calibrationValues.insert(calibrationValues.end(), currentValues.begin(), currentValues.end());
  }

  struct portTotalCount {
    int port;
    double total = 0;
    int count = 0;
  };

  std::vector<portTotalCount> portPercentageAverages(numSensors);

  for (const auto &value : calibrationValues) {
    if (value.redPort != homePort) {
      for (const auto &value2 : calibrationValues) {
        if (value.colour == value2.colour & value2.redPort == homePort) {
          double percentageDiff = calculatePercentageDifference(value.resistanceValue, value2.resistanceValue);
          portPercentageAverages[value.redPort].total += percentageDiff;
          portPercentageAverages[value.redPort].count++;
          portPercentageAverages[value.redPort].port = value.redPort;
        }
      }
    }
  }

  Serial.print("{");
  for (int redPort = 0; redPort < numSensors; redPort++) {
    double averagePercentageDiff = portPercentageAverages[redPort].total / portPercentageAverages[redPort].count * -1 / 100;
    if (isnan(averagePercentageDiff)) {
      Serial.print(0.0);
    } else {
      Serial.print(averagePercentageDiff, 4);
    }
    if (redPort < numSensors - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("};");
}

void calibrateWires() {
  std::vector<wireConnection> calibrationValues;

  boolean firstBlackCalibration = true;
  for (int moveBlackWires = 0; moveBlackWires < numSensors; moveBlackWires++) {

    do {
      if (firstBlackCalibration) {
        Serial.println("Setup the black connections in the natural position RGBY from left to right. Enter ok to continue.");
        firstBlackCalibration = false;
      } else {
        Serial.println("Move each black connection one to the right. Enter ok to continue.");
      }

      while (Serial.available() == 0) {
      }

      String userInput = Serial.readStringUntil('\n');

      if (userInput.equalsIgnoreCase("ok")) {
        Serial.println("Accepted.");
        break;
      } else {
        Serial.println("Invalid input. Please enter 'ok'.");
      }
    } while (true);

    std::vector<wireConnection> currentValues = readCurrentWireSetupCalibration(0, moveBlackWires);
    calibrationValues.insert(calibrationValues.end(), currentValues.begin(), currentValues.end());
  }

  allWireConnections = calibrationValues;

  // std::sort(allWireConnections.begin(), allWireConnections.end(), compareByColourAndBlackPort);
  std::sort(allWireConnections.begin(), allWireConnections.end(), compareByResistance);
  printWireConnections(allWireConnections);
  Serial.println();
  printWireConnectionsRaw(allWireConnections);
}
