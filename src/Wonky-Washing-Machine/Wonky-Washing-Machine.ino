#include <SPI.h>
#include "servo_controller.h"
#include "activation.h"
#include "knobs_game.h"
#include "pattern_game.h"
#include "wires_game.h"
#include "hub_controller.h"

/**
   @brief Instance of 'hubController' for managing network communication and server commands.
*/
HubController hubController;

/**
   @brief Main setup function for the Pattern Wonky-Washing-Machine puzzle.

   This function performs the following tasks:
   - Sets up networking
   - Initializes the random number generator
   - Sets up the knobs game
   - Sets up the pattern game
   - Sets up the wires game
   - Sets up the servos
   - Starts the puzzle
*/
void setup() {

  SPI.begin();

  hubController.hubEnabled = true; // Flag to turn networking on/off.

  Serial.println("Setting up puzzle...");

  Serial.begin(9600); // Serial for local monitoring.
  Serial2.begin(9600); // Serial2 for esp-32 communication.

  randomSeed(analogRead(A0)); // Random seed of empty analog pin for better RNG.

  hubController.setupHub(); // Sertup server hub communication.

  // Set SS pins as OUTPUT and HIGH (off)
  // SPI communication wired on same bus so slave select pins must diable when not in use.
  pinMode(SS_PIN_RFID, OUTPUT);
  delay(100);
  digitalWrite(SS_PIN_RFID, HIGH);
  delay(100);
  pinMode(SS_PIN_MATRIX, OUTPUT);
  delay(100);
  digitalWrite(SS_PIN_MATRIX, HIGH);

  /**
     Initialize the knobs game.
  */
  setupKnobs();

  /**
     Initialize the pattern game.
  */
  setupPatternGame();

  /**
    Initialize the wires game.
  */
  delay(1000);
  digitalWrite(SS_PIN_MATRIX, LOW);
  delay(1000);
  setupWireGame();
  digitalWrite(SS_PIN_MATRIX, HIGH);

  /**
     Initialize the activation sequence.
  */
  delay(500);
  digitalWrite(SS_PIN_RFID, LOW);
  delay(500);
  setupActivation();
  digitalWrite(SS_PIN_RFID, HIGH);

  /**
    Optional safety delay for to wait for button press before moving parts activate.
  */
  //while (checkButtonPressed() < 0);
  //delay(10000);

  /**
     Set up the servos.
  */
  servoControllerSetup();

  Serial.println("----- Puzzle setup complete -----");

  /**
     Run the main puzzle game.
  */
  runPuzzle();
}

/**
   @brief Main idle loop waiting for server commands.
*/
void loop() {
  delay(500);
  hubController.checkHub();
  checkForReset();
}

/**
   @brief Run function for the pattern game.

   This function calls each section of the puzzle sequentially until they are all complete.
   It periodically checks for server commands between sections.
*/
void runPuzzle() {

  Serial.println("Running puzzle...");
  hubController.checkHub();

  /**
     Run activation.
  */
  delay(500);
  digitalWrite(SS_PIN_RFID, LOW);
  delay(500);
  runActivation(hubController);
  digitalWrite(SS_PIN_RFID, HIGH);
  hubController.checkHub();

  /**
      Run knobs game.
  */
  runKnobs(hubController);
  hubController.checkHub();


  /**
      Open door to reveal pattern game.
  */
  openDoor(topServo);
  hubController.checkHub();


  /**
      Run pattern game.
  */
  runPatternGame(hubController);
  hubController.checkHub();

  /**
      Open door to reveal wire game.
  */
  openDoor(sideServo);
  hubController.checkHub();

  /**
     Optional calibration for wires game.
  */
  //calibrateRedPorts();
  //calibrateWires();

  /**
     Run wires game.
  */
  delay(1000);
  digitalWrite(SS_PIN_MATRIX, LOW);
  delay(1000);
  runWiresGame(hubController);
  hubController.checkHub();
  digitalWrite(SS_PIN_MATRIX, HIGH);

  /**
     Open door to reveal final clue.
  */
  openDoor(bottomServo);
  hubController.checkHub();

  Serial.print("Puzzle complete!");
}

/**
   @brief Handle a reset request from the server.

   This must be handled in the top scope for access to runPuzzle().
*/
void checkForReset() {

  if (hubController.hubPuzzleResetRequest) {
    Serial.println("Handling reset request...");
    activationComplete = false;
    knobs_complete = false;
    patternGameComplete = false;
    wiresGameComplete = false;

    hubController.hubPuzzleResetRequest = false;
    runPuzzle();
  }
}
