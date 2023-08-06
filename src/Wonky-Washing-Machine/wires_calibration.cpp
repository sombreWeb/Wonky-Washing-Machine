#include "ArduinoSTL.h"
#include "wire_calibration.h"

const int numSensors = 4;
const int sensorPins[] = { PORT_PIN_0, PORT_PIN_1, PORT_PIN_2, PORT_PIN_3 };
int sensorValues[numSensors] = { 0 };
float Vouts[numSensors] = { 0 };
float Vin = 5;
float Rref = 15000;

int homePort = 0;
const float redPortOffsets[numSensors] = {0.00, 0.0073, 0.0107, 0.0044};

const int numberOfResistanceTests = 100;

std::vector<wireConnection> allWireConnections =
{
  {6970.12f, 0, 'r', 0},
  {8392.12f, 0, 'r', 1},
  {9276.77f, 0, 'r', 2},
  {11380.35f, 0, 'r', 3},
  {12602.50f, 1, 'g', 0},
  {14183.80f, 1, 'g', 1},
  {15100.91f, 1, 'g', 2},
  {17365.66f, 1, 'g', 3},
  {19204.01f, 2, 'b', 0},
  {20821.13f, 2, 'b', 1},
  {21821.16f, 2, 'b', 2},
  {24192.64f, 2, 'b', 3},
  {28839.52f, 3, 'y', 0},
  {30593.86f, 3, 'y', 1},
  {31774.58f, 3, 'y', 2},
  {34466.62f, 3, 'y', 3},
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

    delay(10);  // Delay to allow the ADC to stabilize between readings
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
