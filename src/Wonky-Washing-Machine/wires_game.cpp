#include "wires_game.h"
#include "wire_calibration.h"
#include "ArduinoSTL.h"
#include <SPI.h>
#include <MD_MAX72xx.h>
#include "hub_controller.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define NUM_LED_MATRICES 8

//#define CLK_PIN 13
//#define DATA_PIN 11

/**
   @brief The light intensity for the LED matrices.
*/
const int LIGHT_INTENSITY = 2;

/**
   @brief Represents the LED matrix object.
*/
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, SS_PIN_MATRIX, NUM_LED_MATRICES);

/**
   @brief Tolerance for wire search.

   Will determine how far off a reading can be to the expected wire values to ping a match.
   A higher tolerance will allow the game deviate in readings in a real world environment at a slight accuracy cost.
   Tests proved that this is larely acceptable as there are 64 combinations and higher tollerances at most add a few more solutions.
*/
const float searchTolerance = 500.0f;

/**
   @brief The current level of the wires game.
*/
int wiresGameLevel = 1;

/**
   @brief Flag to indicate if the wires game is complete.
*/
bool wiresGameComplete = false;

/**
   @brief Stores the clue for the wires game.

   Eight characters from top to bottom, left to right on the matrices.
*/
String clue;

/**
   @brief Stores the solution for the wires game.
*/
std::vector<wireConnection> solution;

/**
   @brief Stores the current wire connections.
*/
std::vector<wireConnection> currentWireConnections;

/**
   @brief Initializes the wires game setup, including calibration and LED matrix.
*/
void setupWireGame()
{
  calibrationSetup();
  delay(500);
  mx.begin();
  delay(500);
  mx.control(MD_MAX72XX::INTENSITY, LIGHT_INTENSITY);
  delay(2000);
  showClue("RGBYRGBY");
  delay(1000);
  //updateLiveConnectionOffsets();
  showClue("        ");
}

/**
   @brief Runs the wires game loop, managing clues, actions, and game completion.
   @param hubController The hub controller object.
*/
void runWiresGame(HubController &hubController)
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

  LevelData levelData = getLevelData(wiresGameLevel);
  solution = levelData.levelSolution;
  clue = levelData.clue;

  // Diable colour clues for level one
  if (wiresGameLevel == 1) {
    colourClueEnabled = false;
  }

  showClue(clue);

  while (!wiresGameComplete) {

    hubController.checkHub();

    currentMillis = millis();

    printWireConnections(currentWireConnections);
    std::vector<wireConnection> currentConnections = readCurrentWireSetupCalibration(0, 0);
    Serial.println("-------- Current Readings: (red ports only) ---------");
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
    if (!wiresGameComplete) {
      wiresGameComplete = checkIfWiresCorrect();
    }
  }

  if (wiresGameComplete) {
    showClue("********");
  }

}

/**
   @brief Shows a clue on the LED matrix.
   @param clue The clue to display.
*/
void showClue(String clue) {
  String text = clue.substring(4, 8) + clue.substring(0, 4);
  mx.clear();
  int printIndex = (clue.length() * 8) - 3;
  for (char currentChar : text) {
    mx.setChar(printIndex, currentChar);
    printIndex -= 8;
  }
}

/**
   @brief Compares two wireConnection objects by their red port.
   @param connection1 The first wireConnection object.
   @param connection2 The second wireConnection object.
   @return True if the red port of connection1 is less than the red port of connection2; otherwise, false.
*/
bool compareByRedPort(const wireConnection& connection1, const wireConnection& connection2) {
  return connection1.redPort < connection2.redPort;
}

/**
   @brief Compares two wireConnection objects by their black port.
   @param connection1 The first wireConnection object.
   @param connection2 The second wireConnection object.
   @return True if the black port of connection1 is less than the black port of connection2; otherwise, false.
*/
bool compareByBlackPort(const wireConnection& connection1, const wireConnection& connection2) {
  return connection1.blackPort < connection2.blackPort;
}

/**
   @brief Checks if the current wire connections match the solution.

   This function checks for exclusive matches and will not accept multiple readings for one wire colour.
   More accurate but less viable in real world environment.

   @return True if the wire connections match the solution; otherwise, false.
*/
boolean checkIfWiresCorrectExclusiveColours() {
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

/**
   @brief Checks if the current wire connections match the solution.

   Will read the wires and be satisfied at any match for each of the colours.
   Ignores double reads on the same colour.
   Proven better in testing for real world hardware variance.

   @return True if the wire connections match the solution; otherwise, false.
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

  return true;
}

/**
   @brief Retrieves level-specific data for the wires game.
   @param level The level for which to get data.
   @return The LevelData structure containing level-specific data.
*/
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
      // solve message: C8B6 I34G +1
      levelData.clue = "C8B6I34G";
      levelData.levelSolution =
      {
        {0, 0, 'r', 1},
        {0, 1, 'y', 2},
        {0, 2, 'g', 0},
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

/**
   @brief Filters the closest color values from a list of connections.

   Can be used if a single colour only is desired from readings.
   The colour connection will be the one which is closest to a matching external resistance value.

   @param connections The list of wire connections.
   @param externalAverageResistance The external average resistance value for filtering.
   @return A vector containing the filtered wire connections.
*/
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

/**
   @brief Retrieves the current wire connections based on sensor readings.
   @return A vector containing the current wire connections.
*/
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

/**
   @brief Flashes the colors of the solution on the LED matrix.

   This gives the players an indication as to the order of the colours - but not the actual connection solution.

   @param solutionToFlash The solution wire connections to flash.
*/
void flashColours(std::vector<wireConnection> solutionToFlash) {

  std::sort(solutionToFlash.begin(), solutionToFlash.end(), compareByBlackPort);

  String text = "";
  for (const auto &solution : solutionToFlash) {
    text.concat(solution.colour);
  }

  text.toUpperCase();

  showClue(text);
}
