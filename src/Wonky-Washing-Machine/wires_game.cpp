#include "wires_game.h"
#include "wire_calibration.h"
#include "ArduinoSTL.h"

const float searchTolerance = 1000.0f;

int wiresGameLevel = 1;

bool wiresGameComplete = false;

std::vector<wireConnection> solution;
std::vector<wireConnection> currentWireConnections;

void setupWireGame()
{
  solution = getSolution(wiresGameLevel);
}

void runWiresGame()
{
  while (!wiresGameComplete) {
    wiresGameComplete = checkIfWiresCorrect();
    printWireConnections(currentWireConnections);
  }
}

bool compareByRedPort(const wireConnection& connection1, const wireConnection& connection2) {
  return connection1.redPort < connection2.redPort;
}

boolean checkIfWiresCorrect() {
  currentWireConnections = getCurrentConnections();

  std::sort(currentWireConnections.begin(), currentWireConnections.end(), compareByRedPort);
  std::sort(solution.begin(), solution.end(), compareByRedPort);

  if (currentWireConnections.size() < numSensors) {
    return false;
  }

  for (int connectionIndex = 0; connectionIndex < numSensors; connectionIndex++) {
    if (currentWireConnections[connectionIndex].redPort == solution[connectionIndex].redPort &&
        currentWireConnections[connectionIndex].blackPort == solution[connectionIndex].blackPort &&
        currentWireConnections[connectionIndex].colour == solution[connectionIndex].colour) {
      continue;
    } else {
      return false;
    }
  }

  Serial.println("correct");
  return true;
}

std::vector<wireConnection> getSolution(int level) {
  std::vector<wireConnection> levelSolution;
  switch (level) {
    case 1:
      // solve message: BYRG RYGB
      levelSolution =
      {
        {0, 0, 'b', 3},
        {0, 1, 'y', 1},
        {0, 2, 'r', 0},
        {0, 3, 'g', 2}
      };
      break;
    case 2:
      // solve message: 5927 BGEI
      levelSolution =
      {
        {0, 0, 'g', 2},
        {0, 1, 'r', 3},
        {0, 2, 'y', 0},
        {0, 3, 'b', 1}
      };
      break;
    case 3:
      // solve message: C9B6 H3DE +-1
      levelSolution =
      {
        {0, 0, 'r', 2},
        {0, 1, 'y', 0},
        {0, 2, 'g', 1},
        {0, 3, 'b', 3}
      };
      break;
    default:
      // solve message: BYRG RYGB
      levelSolution =
      {
        {0, 0, 'b', 3},
        {0, 1, 'y', 1},
        {0, 2, 'r', 0},
        {0, 3, 'g', 2}
      };
      break;
  }
  return levelSolution;
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

std::vector<wireConnection> getCurrentConnections() {
  std::vector<wireConnection> currentConnections;
  for (int sensorNum = 0; sensorNum < numSensors; sensorNum++) {
    float averageRestistance = findAverageResistance(sensorNum, numberOfResistanceTests);
    for (const auto &values : allWireConnections) {
      if (fabs(averageRestistance - values.resistanceValue) <= searchTolerance) {
        currentConnections.push_back({averageRestistance, sensorNum, values.colour, values.blackPort});
      }
    }
    //currentConnections = filterClosestColourValues(currentConnections, averageRestistance);
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
