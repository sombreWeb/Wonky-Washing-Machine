#include "wires_game.h"
#include "wire_calibration.h"
#include "ArduinoSTL.h"

const float searchTolerance = 1500.0f;

void setupWireGame()
{
  std::vector<wireConnection> currentWireConnections;
  currentWireConnections = getCurrentConnections();
  printWireConnections(currentWireConnections);
  //calibrateWires();
}

void runWireGame()
{
}

std::vector<wireConnection> getCurrentConnections() {
  std::vector<wireConnection> currentConnections;
  for (int sensorNum = 0; sensorNum < numSensors; sensorNum++) {
    float averageRestistance = findAverageResistance(sensorNum, numberOfResistanceTests);
    for (const auto &values : allWireConnections) {
      if (fabs(averageRestistance - values.resistanceValue) <= searchTolerance) {
        currentConnections.push_back({averageRestistance, sensorNum, values.colour, values.blackPort});
      }
    }
    currentConnections = filterClosestColourValues(currentConnections, averageRestistance);
  }

  std::vector<wireConnection> currentConnectionsFiltered;
  const char colours[] = {'r', 'g', 'b', 'y'};
  for (char colour : colours) {
    for (const auto &connection : currentConnections) {
      if (connection.colour == colour) {
        currentConnectionsFiltered.push_back(connection);
      }
    }
  }

  return currentConnectionsFiltered;
}

std::vector<wireConnection> filterClosestColourValues(std::vector<wireConnection> &connections, float externalAverageResistance) {

  struct uniqueColourMatches {
    char colour;
    wireConnection connection;
  };

  std::vector<uniqueColourMatches> closestElements;

  for (const auto &connection : connections) {
    char colour = connection.colour;
    float distanceToAverage = fabs(connection.resistanceValue - externalAverageResistance);
    if (closestElements[colour].colour == '\0' || distanceToAverage < fabs(closestElements[colour].connection.resistanceValue - externalAverageResistance)) {
      closestElements[colour] = {colour, connection};
    }
  }

  std::vector<wireConnection> filteredConnections;
  const char colours[] = {'r', 'g', 'b', 'y'};
  for (char colour : colours) {
    filteredConnections.push_back(closestElements[colour].connection);
  }

  return filteredConnections;
}
