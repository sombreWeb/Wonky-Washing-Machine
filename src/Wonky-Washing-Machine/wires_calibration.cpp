#include "ArduinoSTL.h"
#include "wire_calibration.h"
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

/**
   @brief Number of sensors and their respective pins.
*/
const int numSensors = 4;
const int sensorPins[] = { PORT_PIN_0, PORT_PIN_1, PORT_PIN_2, PORT_PIN_3 };

/**
   @brief Arrays for storing sensor values and Vouts.
*/
int sensorValues[numSensors] = { 0 };
double Vouts[numSensors] = { 0 };

/**
   @brief Constants for voltage and reference resistance.
*/
float Vin = 5;
float Rref = 15000;

/**
   @brief ADS1115 ADC object for anlog readings.
*/
Adafruit_ADS1115 ads;

/**
   @brief Maximum allowed resistance value.
*/
const float MAX_ALLOWED_RESISTANCE_VALUE = 100000;

/**
   @brief Home port for reference when calibrating other ports to match.
*/
int homePort = 0;

/**
   @brief Red port offsets to make all meaure equally.
*/
const float redPortOffsets[numSensors] = {0.00, 0.0052, 0.0003, -0.0031};

/**
   @brief Live connection offsets to store temporary calibration values if enabled.
*/
float liveConnectionOffsets[numSensors] = {0.00, 0.0, 0.0, 0.0};

/**
   @brief Number of tests to average to get a resistance value.
*/
const int numberOfResistanceTests = 5;

/**
   @brief Initializes the calibration setup.

   Initializes the ADS1115 ADC and sets its gain.
*/
void calibrationSetup() {
  ads.setGain(GAIN_ONE);
  ads.begin();
}

/**
   @brief A lookup vector to match read values to expected connection values.

   This assumes all red port measure the same through calibration allowing all 64 combinations to be determined.
*/
std::vector<wireConnection> allWireConnections =
{
  {3019.72f, 0, 'r', 0},
  {3500.91f, 0, 'r', 1},
  {3800.54f, 0, 'r', 2},
  {4444.18f, 0, 'r', 3},

  {4800.60f, 1, 'g', 0},
  {5196.37f, 1, 'g', 1},
  {5430.47f, 1, 'g', 2},
  {5969.80f, 1, 'g', 3},

  {6378.69f, 2, 'b', 0},
  {6707.95f, 2, 'b', 1},
  {6910.22f, 2, 'b', 2},
  {7341.59f, 2, 'b', 3},

  {8100.43f, 3, 'y', 0},
  {8349.52f, 3, 'y', 1},
  {8510.75f, 3, 'y', 2},
  {8844.52f, 3, 'y', 3},
};

/**
   @brief Optional funtion to quickly read the sensors of a single wire reading.

   Used for quick calibration before runs.

   @note This feature is not generally necessary as the live system tested stable without it.
   @note Feature not used in live system.
*/
void updateLiveConnectionOffsets() {
  std::vector<wireConnection> currentLiveConnections = readCurrentWireSetupCalibration(0, 0);

  for (int i = 0; i < currentLiveConnections.size(); ++i) {

    if ( currentLiveConnections[i].resistanceValue > MAX_ALLOWED_RESISTANCE_VALUE) {
      liveConnectionOffsets[i] = 0.0;
      continue;
    }

    if (currentLiveConnections[i].redPort == allWireConnections[(i * numSensors) + i].redPort &&
        currentLiveConnections[i].colour == allWireConnections[(i * numSensors) + i].colour &&
        currentLiveConnections[i].blackPort == allWireConnections[(i * numSensors) + i].blackPort) {

      float diff = allWireConnections[(i * numSensors) + i].resistanceValue - currentLiveConnections[i].resistanceValue;
      liveConnectionOffsets[i] = diff;
    } else {
      liveConnectionOffsets[i] = 0.0;
    }
  }

  applyLiveConnectionOffsets();

  for (const auto& connection : allWireConnections) {
    Serial.print("Resistance: ");
    Serial.print(connection.resistanceValue);
    Serial.print(", Red Port: ");
    Serial.print(connection.redPort);
    Serial.print(", Colour: ");
    Serial.print(connection.colour);
    Serial.print(", Black Port: ");
    Serial.println(connection.blackPort);
  }
}

/**
   @brief Optional funtion to apply live connection offset readings.

   Used for quick calibration before runs.

   @note This feature is not generally necessary as the live system tested stable without it.
   @note Feature not used in live system.
*/
void applyLiveConnectionOffsets() {
  for (auto& connection : allWireConnections) {
    if (connection.colour == 'r') {
      connection.resistanceValue += liveConnectionOffsets[0];
    } else if (connection.colour == 'g') {
      connection.resistanceValue += liveConnectionOffsets[1];
    } else if (connection.colour == 'b') {
      connection.resistanceValue += liveConnectionOffsets[2];
    } else if (connection.colour == 'y') {
      connection.resistanceValue += liveConnectionOffsets[3];
    }
  }
}

/**
   @brief Compares two wireConnection objects by color and black port.

   This comparator function is used to sort wireConnection objects primarily by color
   and secondarily by black port.

   @param wire1 The first wireConnection object to compare.
   @param wire2 The second wireConnection object to compare.

   @return True if wire1 should come before wire2 in the sorted order; otherwise, false.
*/
bool compareByColourAndBlackPort(const wireConnection &wire1, const wireConnection &wire2) {
  if (wire1.colour != wire2.colour) {
    return wire1.colour < wire2.colour;
  } else {
    return wire1.blackPort < wire2.blackPort;
  }
}

/**
   @brief Compares two wireConnection objects by color and red port.

   This comparator function is used to sort wireConnection objects primarily by color
   and secondarily by red port.

   @param wire1 The first wireConnection object to compare.
   @param wire2 The second wireConnection object to compare.

   @return True if wire1 should come before wire2 in the sorted order; otherwise, false.
*/
bool compareByColourAndRedPort(const wireConnection &wire1, const wireConnection &wire2) {
  if (wire1.colour != wire2.colour) {
    return wire1.colour < wire2.colour;
  } else {
    return wire1.redPort < wire2.redPort;
  }
}

/**
   @brief Compares two wireConnection objects by their resistance values.

   This comparator function is used to sort wireConnection objects based on their
   resistance values.

   @param wire1 The first wireConnection object to compare.
   @param wire2 The second wireConnection object to compare.

   @return True if wire1's resistance value is less than wire2's; otherwise, false.
*/
bool compareByResistance(const wireConnection &wire1, const wireConnection &wire2) {
  return wire1.resistanceValue < wire2.resistanceValue;
}

/**
   @brief Calculates the percentage difference between two values.

   @param value1 The first value.
   @param value2 The second value.

   @return The percentage difference between value1 and value2.
*/
double calculatePercentageDifference(double value1, double value2) {
  double percentageDiff = ((value1 - value2) / ((value1 + value2) / 2.0)) * 100.0;
  if (isnan(percentageDiff)) {
    return 0.0;
  }
  return percentageDiff;
}


/**
   @brief Finds the average resistance value for a sensor.

   The red port offset can be disabled to allow red port calibration.

   @param sensorNumber The sensor number.
   @param numberOfTests The number of resistance tests to perform.
   @param redOffsetOn Whether to apply red port offset.

   @return The average resistance value for the sensor.
*/
float findAverageResistance(int sensorNumber, int numberOfTests, boolean redOffsetOn = true) {
  float redOffset = Rref * (1 + redPortOffsets[sensorNumber]);

  float sumResistance = 0;
  for (int j = 0; j < numberOfTests; j++) {
    //sensorValues[sensorNumber] = analogRead(sensorPins[sensorNumber]);
    sensorValues[sensorNumber] = ads.readADC_SingleEnded(sensorNumber);
    //Vouts[sensorNumber] = (Vin * sensorValues[sensorNumber]) / 1023.0;
    Vouts[sensorNumber] = (Vin / 65535.0) * sensorValues[sensorNumber];
    float resistance = Rref * (1.0 / ((Vin / Vouts[sensorNumber]) - 1.0));
    sumResistance += resistance;

    //delay(10);  // Delay to allow the ADC to stabilize between readings
  }
  float averageResistance = sumResistance / numberOfTests;  // Calculate average resistance
  if (redOffsetOn) {
    averageResistance = averageResistance * (1 + redPortOffsets[sensorNumber]);
  }
  return averageResistance;
}

/**
   @brief Reads the current wire setup for calibration.

   Differs from readCurrentWireSetup in main as this function expects the wires to be in a paticular
   order: R G B Y - with optional offsets to manipulate their expected positions from that order.

   @param redOffset The red port offset.
   @param blackOffset The black port offset.
   @param redOffsetOn Whether to apply red port offset.

   @return A vector of wireConnection objects representing the current wire setup calibration.
*/
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

/**
   @brief Prints the wire connections to the serial monitor.

   @param values A vector of wireConnection objects to print.
*/
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

/**
   @brief Prints the wire connections to the serial monitor in raw format.

   This output can then be used to replace the current allWireConnections vectors if desired.

   @param values A vector of wireConnection objects to print.
*/
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


/**
   @brief Prints the current wire connections to the serial monitor.
*/
void printCurrentWireConnections() {
  std::vector<wireConnection> currentValues = readCurrentWireSetupCalibration(0, 0);
  std::sort(currentValues.begin(), currentValues.end(), compareByColourAndBlackPort);
  printWireConnections(currentValues);
}

/**
   @brief Performs the red ports calibration.
*/
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


/**
   @brief Performs the wire calibration.
*/
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
