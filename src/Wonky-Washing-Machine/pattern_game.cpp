#include "pattern_game.h"
#include <FastLED.h> // FastLED library for controlling LEDs
#include "hub_controller.h"

/**
   Level for the pattern game.
*/
int patternGameLevel = 1;

/**
   Flag to track the completion of the pattern game.
*/
bool patternGameComplete = false;

/**
  Value to set brightness of LEDs (0-255).
*/
const int LED_BRIGHTNESS_PATTERN_GAME = 150;

/**
   Length of the patterns per level.
*/
const int patternLengthLevel1 = 4;
const int patternLengthLevel2 = 6;
const int patternLengthLevel3 = 8;

/**
   Increment of the patterns per level.
*/
const int patternIncrementLevel1 = 1;
const int patternIncrementLevel2 = 2;
const int patternIncrementLevel3 = 3;

/**
   Speed of the patterns per level.
*/
const int patternSpeedLevel1 = 1000;
const int patternSpeedLevel2 = 1000;
const int patternSpeedLevel3 = 750;

/**
   Hex values for colours.
*/
const unsigned long RED_HEX = 0xFF0000;
const unsigned long GREEN_HEX = 0x00FF00;
const unsigned long BLUE_HEX = 0x0000FF;
const unsigned long YELLOW_HEX = 0xF6FF00;
const unsigned long WHITE_HEX = 0xFFFFFF;

/**
   Time in miliseconds to wait for another button press.
*/
const int BUTTON_PRESS_WAIT_TIME = 3000;

int lastButtonPressed = -1;

/**
   Array to store the LED colours.
*/
CRGB leds[NUM_LEDS];

/**
   Setup function for the pattern game.
*/
void setupPatternGame()
{
  pinMode(BUTTON_PIN_RED, INPUT);
  pinMode(BUTTON_PIN_GREEN, INPUT);
  pinMode(BUTTON_PIN_BLUE, INPUT);
  pinMode(BUTTON_PIN_YELLOW, INPUT);

  FastLED.addLeds<NEOPIXEL, LEDS_DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS_PATTERN_GAME);

  fillLights("black");

  delay(500);
}

/**
   Function to run the pattern game.
*/
void runPatternGame(HubController &hubController)
{
  fillLights("black");
  
  // Set game level
  int patternLength, patternIncrement, patternSpeed;

  switch (patternGameLevel)
  {
    case 1:
      patternLength = patternLengthLevel1;
      patternIncrement = patternIncrementLevel1;
      patternSpeed = patternSpeedLevel1;
      break;
    case 2:
      patternLength = patternLengthLevel2;
      patternIncrement = patternIncrementLevel2;
      patternSpeed = patternSpeedLevel2;
      break;
    case 3:
      patternLength = patternLengthLevel3;
      patternIncrement = patternIncrementLevel3;
      patternSpeed = patternSpeedLevel3;
      break;
    default:
      patternLength = patternLengthLevel1;
      patternIncrement = patternIncrementLevel1;
      patternSpeed = patternSpeedLevel1;
  }

  // Generate a random pattern
  int *randomPattern = generateRandomPatternTop(patternLength);

  // Loop until the game is complete
  while (!patternGameComplete)
  {
    hubController.checkHub();
    
    // Loop through the game in increments to give it the progressive completion effect
    for (int patternSegment = 2; patternSegment < (patternLength + patternIncrement); patternSegment += patternIncrement)
    {
      hubController.checkHub();
      if (patternGameComplete){
          break;
        }

     bool segmentComplete = false; // Flag to check if this segment of the game is complete

      // Loop over this segment repeatedly until the player completes the segment
      while (!segmentComplete)
      {
        hubController.checkHub();
        if (patternGameComplete){
          break;
        }

        // Display the colours to the player
        for (int colourIndex = 0; colourIndex < (patternSegment + 1); colourIndex++) // Plus one for the white flash
        {
          if (segmentComplete) // Stop looping if this segment is complete
          {
            break;
          }

          // Set the colour
          unsigned long colour;
          switch (randomPattern[colourIndex])
          {
            case 0:
              colour = RED_HEX;
              break;
            case 1:
              colour = GREEN_HEX;
              break;
            case 2:
              colour = BLUE_HEX;
              break;
            case 3:
              colour = YELLOW_HEX;
              break;
            default:
              colour = WHITE_HEX;
              break;
          }

          setLEDTop(randomPattern[colourIndex], colour); // Display the colour on the LEDs

          if (colourIndex == patternSegment) // Do a white flash at the end of the pattern
          {
            fillLights("white");
          }

          // Loop to delay LED turning off. Allows continuous checking for a button press to interrupt lights flashing
          for (int lightDelayIncrement = 0; lightDelayIncrement < patternSpeed; lightDelayIncrement++)
          {

            if (segmentComplete) // Stop looping if this segment is complete
            {
              break;
            }

            // Create an array to store the sequence of pressed buttons. Initialise them all to -1
            int buttonsPressed[patternLength];
            for (int buttonIdx = 0; buttonIdx < patternLength; buttonIdx++)
            {
              buttonsPressed[buttonIdx] = -1;
            }

            // Loop to check for a button press. This loop allows us to reset the timer between button presses on continue
            while (true)
            {

              // check if button pressed
              if (checkButtonPressed() > -1)
              {
                // add to buttons pressed queue
                updateButtonQueue(buttonsPressed, patternLength);

                // turn off lights
                fillLights("black");

                // Loop to enter button pressed mode. Stop showing the player the pattern and wait until a set time for another press
                for (int buttonDelayIncrement = 0; buttonDelayIncrement < BUTTON_PRESS_WAIT_TIME; buttonDelayIncrement++)
                {
                  if (segmentComplete) // Stop looping if this segment is complete
                  {
                    break;
                  }

                  if (checkButtonPressed() > -1) // If a button is pressed start the timer again and wait for another
                  {
                    updateButtonQueue(buttonsPressed, patternLength);
                    continue;
                  }

                  // Check if the segment pattern was completed successfully
                  if (compareArrays(&(buttonsPressed[patternLength - patternSegment]), randomPattern, patternSegment))
                  { // the subtraction here allows a dynamic start point for the buttons pressed array pointer which adds to the end of the queue
                    segmentComplete = true;
                    delay(patternSpeed); // A delay to stop the next segment starting immediately
                  }

                  delay(1); // Delay between button checks
                }
              }
              else
              {
                break; // Break from button pressed mode if no button was pressed in the time limit
              }
            }

            delay(1); // Delay light display time
          }
        }
      }
    }
    
    patternGameComplete = true; // End the game
  }

  if (patternGameComplete){
    fillLights("green"); // Fill LEDs to green when the pattern game was completed successfully
  }

  delete[] randomPattern; // Release the memory allocated for the random pattern array
}

/**
   Function to set the colour of a specific LED.

   @param ledNum The LED number.
   @param colour The colour to set (in hex format).
*/
void setLEDTop(int ledNum, unsigned long colour)
{
  if (ledNum >= 0 && ledNum < NUM_LEDS) // Check if the LED number is within the valid range
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Turn off all LEDs
    leds[ledNum] = colour; // Set the specified LED to the specified colour
    FastLED.show(); // Update the LED strip to show the changes
  }
}

/**
   Function to set all LEDs to a defined colour.
*/
void fillLights(String colour)
{
  if (colour == "white")
  {
    fill_solid(leds, NUM_LEDS, CRGB::White); // Set all LEDs to white
  }
  else if (colour == "black")
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Set all LEDs to black (off)
  }
  else if (colour == "green")
  {
    fill_solid(leds, NUM_LEDS, CRGB::Green); // Set all LEDs to green
  }
  else
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Default: Set all LEDs to black (off)
  }
  FastLED.show(); // Update the LED strip to show the changes
}

/**
   Function to check which button is pressed.

   @return The button number (0-3) or -1 if no button is pressed.
*/
int checkButtonPressed()
{

  if (digitalRead(BUTTON_PIN_RED) == HIGH)
  {
    Serial.println("Red pressed");
    return 0; // Red button pressed
  }
  else if (digitalRead(BUTTON_PIN_GREEN) == HIGH)
  {
     Serial.println("Green pressed");
    return 1; // Green button pressed
  }
  else if (digitalRead(BUTTON_PIN_BLUE) == HIGH)
  {
     Serial.println("Blue pressed");
    return 2; // Blue button pressed
  }
  else if (digitalRead(BUTTON_PIN_YELLOW) == HIGH)
  {
     Serial.println("Yellow pressed");
    return 3; // Yellow button pressed
  }
  return -1; // No button pressed
}

/**
   Function to update the button queue.

   @param buttonQueue The button queue array.
*/
void updateButtonQueue(int buttonQueue[], int buttonQueueLength)
{
  Serial.println("Updating buttonQueue");
  int latest = checkButtonPressed();
  if (latest >= 0 && latest != buttonQueue[buttonQueueLength - 1]) // Check if a new button press is detected and it's different from the last button in the queue
  {
    addToButtonSequence(buttonQueue, buttonQueueLength, latest); // Add the latest button press to the button sequence
  }
}

/**
   Function to generate a random pattern.

   @param patternLength The length of the pattern.
   @return An array containing the random pattern.
*/
int *generateRandomPatternTop(int patternLength)
{
  int *pattern = new int[patternLength]; // Array to store the random pattern
  int previousNumber = -1; // Initialize the previous number to an invalid value

  for (int i = 0; i < patternLength; i++)
  {
    int randomNumber;
    do
    {
      randomNumber = random(NUM_LEDS); // Generate a random number between 0 and NUM_LEDS-1
    } while (randomNumber == previousNumber); // Ensure the current random number is different from the previous one

    pattern[i] = randomNumber; // Store the random number in the pattern array
    previousNumber = randomNumber; // Update the previous number for the next iteration
  }

  return pattern;
}

/**
   Function to add a value to the button sequence.

   @param buttonSequence The button sequence array.
   @param buttonSequenceLength The length of the button array.
   @param value The value to add.
*/
void addToButtonSequence(int buttonSequence[], int buttonSequenceLength, int value)
{
  for (int i = 0; i < buttonSequenceLength - 1; i++)
  {
    buttonSequence[i] = buttonSequence[i + 1]; // Shift the elements in the button sequence array to the left
  }

  Serial.println("Added to button queue");
  buttonSequence[buttonSequenceLength - 1] = value; // Add the new value to the end of the button sequence
}

/**
   Function to compare two arrays.

   @param array1 The first array.
   @param array2 The second array.
   @param arrayLength The length of the arrays.
   @return True if the arrays are equal, false otherwise.
*/
bool compareArrays(const int *array1, const int *array2, int arrayLength)
{
  for (int i = 0; i < arrayLength; i++)
  {
    if (array1[i] != array2[i]) // Compare each element of the arrays
    {
      return false; // Arrays are not equal
    }
  }
  return true; // Arrays are equal
}
