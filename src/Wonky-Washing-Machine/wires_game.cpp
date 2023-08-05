#include "wires_game.h"
#include "wire_calibration.h"
#include "ArduinoSTL.h"
#include <SPI.h>
#include <MD_MAX72xx.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define NUM_LED_MATRICES 8

//#define CLK_PIN 13
//#define DATA_PIN 11

const int LIGHT_INTENSITY = 1;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, SS_PIN_MATRIX, NUM_LED_MATRICES);

const float searchTolerance = 1000.0f;

int wiresGameLevel = 1;

bool wiresGameComplete = false;

String clue;
std::vector<wireConnection> solution;
std::vector<wireConnection> currentWireConnections;


void setupWireGame()
{
  LevelData levelData = getLevelData(wiresGameLevel);
  solution = levelData.levelSolution;
  clue = levelData.clue;
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, LIGHT_INTENSITY);
  delay(500);
}

void runWiresGame()
{
  boolean displayBaseClueFlag = true;
  unsigned long baseClueDuration = 15000;
  unsigned long colourClueDuration = 3000;
  unsigned long currentMillis = 0;
  boolean colourClueEnabled = true;
  unsigned long actionStartTime = 0;
  boolean isFirstAction = true;
  boolean lastActionWasShowClue = false;
  boolean lastActionWasFlashColours = false;

  //Temporary
  if (wiresGameLevel == 1) {
    colourClueEnabled = false;
  }

  showClue(clue);

  while (!wiresGameComplete) {
    currentMillis = millis();

    printWireConnections(currentWireConnections);
    std::vector<wireConnection> currentConnections = readCurrentWireSetupCalibration(0, 0);
    Serial.println("--------\\\///---------");
    printWireConnections(currentConnections);


    if (colourClueEnabled) {
      if (displayBaseClueFlag) {
        if (currentMillis - actionStartTime >= baseClueDuration) {
          displayBaseClueFlag = false;
          actionStartTime = currentMillis;
          flashColours(solution);
          lastActionWasFlashColours = true;
          lastActionWasShowClue = false;
          Serial.println("1");
        } else {
          if (!lastActionWasShowClue) {
            showClue(clue);
            lastActionWasShowClue = true;
            lastActionWasFlashColours = false;
            Serial.println("2");
          }
        }
      } else {
        if (currentMillis - actionStartTime >= colourClueDuration) {
          displayBaseClueFlag = true;
          actionStartTime = currentMillis;
          if (!lastActionWasShowClue) {
            showClue(clue);
            lastActionWasShowClue = true;
            lastActionWasFlashColours = false;
            Serial.println("3");
          }
        } else {
          if (!lastActionWasFlashColours) {
            flashColours(solution);
            lastActionWasFlashColours = true;
            lastActionWasShowClue = false;
            Serial.println("4");
          }
        }
      }
    }

    wiresGameComplete = checkIfWiresCorrect();
  }
}

void showClue(String clue) {
  String text = clue.substring(4, 8) + clue.substring(0, 4);
  mx.clear();
  int printIndex = (clue.length() * 8) - 3;
  for (char currentChar : text) {
    mx.setChar(printIndex, currentChar);
    printIndex -= 8;
  }
}

bool compareByRedPort(const wireConnection& connection1, const wireConnection& connection2) {
  return connection1.redPort < connection2.redPort;
}

bool compareByBlackPort(const wireConnection& connection1, const wireConnection& connection2) {
  return connection1.blackPort < connection2.blackPort;
}

/*
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
  showClue("********");

  return true;
  }
*/

bool checkIfWiresCorrect() {

  currentWireConnections = getCurrentConnections();

  for (const wireConnection& solutionConnection : solution) {

    bool foundMatch = false;

    for (const wireConnection& currentConnection : currentWireConnections) {
      if (currentConnection.redPort == solutionConnection.redPort &&
          currentConnection.blackPort == solutionConnection.blackPort &&
          currentConnection.colour == solutionConnection.colour) {

        foundMatch = true;
        break;
      }
    }

    if (!foundMatch) {
      return false;
    }
  }

  Serial.println("correct");
  showClue("********");

  return true;
}


LevelData getLevelData(int level) {
  LevelData levelData;
  switch (level) {
    case 1:
      // solve message: BYRG RYGB
      levelData.clue = "BYRGRYGB";
      levelData.levelSolution =
      {
        {0, 0, 'r', 2},
        {0, 1, 'y', 1},
        {0, 2, 'g', 3},
        {0, 3, 'b', 0}
      };
      break;
    case 2:
      // solve message: 5927 BGEI
      levelData.clue = "5927BGEI";
      levelData.levelSolution =
      {
        {0, 0, 'g', 2},
        {0, 1, 'r', 3},
        {0, 2, 'y', 0},
        {0, 3, 'b', 1}
      };
      break;
    case 3:
      // solve message: C9B6 H3DE +-1
      levelData.clue = "C9B6H3DE";
      levelData.levelSolution =
      {
        {0, 0, 'g', 1},
        {0, 1, 'r', 2},
        {0, 2, 'y', 0},
        {0, 3, 'b', 3}
      };
      break;
    default:
      // solve message: BYRG RYGB
      levelData.clue = "BYRGRYGB";
      levelData.levelSolution =
      {
        {0, 0, 'r', 2},
        {0, 1, 'y', 1},
        {0, 2, 'g', 3},
        {0, 3, 'b', 0}
      };
      break;
  }
  return levelData;
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

void flashColours(std::vector<wireConnection> solutionToFlash) {

  std::sort(solutionToFlash.begin(), solutionToFlash.end(), compareByBlackPort);

  String text = "";
  for (const auto &solution : solutionToFlash) {
    text.concat(solution.colour);
  }

  text.toUpperCase();

  showClue(text);
}
