#include <FastLED.h>
#include <Servo.h>

#define LEDS_DATA_PIN 7
#define NUM_LEDS 4

#define BUTTON_PIN_RED 8
#define BUTTON_PIN_GREEN 9
#define BUTTON_PIN_BLUE 10
#define BUTTON_PIN_YELLOW 11

int level = 3;

/**
   Flag to track the completion of the pattern game.
*/
bool patternGameComplete = false;

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
const int patternSpeedLevel3 = 1000;

/**
   Hex values for colours.
*/
const unsigned long RED_HEX = 0xFF0000;
const unsigned long GREEN_HEX = 0x00FF00;
const unsigned long BLUE_HEX = 0x0000FF;
const unsigned long YELLOW_HEX = 0xF6FF00;
const unsigned long WHITE_HEX = 0xFFFFFF;

/**
   Time in miliseconds to wait for another button press
*/
const int BUTTON_PRESS_WAIT_TIME = 3000;

/**
   Array to store the random pattern.
*/
// int* randomPattern = generateRandomPatternTop(patternLength);

/**
   Array to store the button queue.
*/
// int buttonQueue[patternLength];

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
  FastLED.setBrightness(10);
}

/**
   Function to set the colour of a specific LED.

   @param ledNum The LED number.
   @param colour The colour to set (in hex format).
*/
void setLEDTop(int ledNum, unsigned long colour)
{
  if (ledNum >= 0 && ledNum < NUM_LEDS)
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[ledNum] = colour;
    FastLED.show();
  }
}

/**
   Function to set all LEDs to a defined colour.
*/
void fillLights(String colour)
{
  if (colour == "white")
  {
    fill_solid(leds, NUM_LEDS, CRGB::White);
  }
  else if (colour == "black")
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  else if (colour == "green")
  {
    fill_solid(leds, NUM_LEDS, CRGB::Green);
  }
  else
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  FastLED.show();
}

/**
   Function to check which button is pressed.

   @return The button number (0-3) or -1 if no button is pressed.
*/
int checkButtonPressed()
{

  if (digitalRead(BUTTON_PIN_RED) == HIGH)
  {
    return 0;
  }
  else if (digitalRead(BUTTON_PIN_GREEN) == HIGH)
  {
    return 1;
  }
  else if (digitalRead(BUTTON_PIN_BLUE) == HIGH)
  {
    return 2;
  }
  else if (digitalRead(BUTTON_PIN_YELLOW) == HIGH)
  {
    return 3;
  }
  return -1; // No button pressed
}

/**
   Function to update the button queue.

   @param buttonQueue The button queue array.
*/
void updateButtonQueue(int buttonQueue[], int buttonQueueLength)
{
  int latest = checkButtonPressed();
  if (latest >= 0 && latest != buttonQueue[buttonQueueLength - 1])
  {
    addToButtonSequence(buttonQueue, buttonQueueLength, latest);

    switch (latest)
    {
      case 0:
        Serial.println("Red");
        break;
      case 1:
        Serial.println("Green");
        break;
      case 2:
        Serial.println("Blue");
        break;
      case 3:
        Serial.println("Yellow");
        break;
      default:
        Serial.println("Invalid input");
        break;
    }
  }
}

/**
   Function to generate a random pattern.

   @param patternLength The length of the pattern.
   @return An array containing the random pattern.
*/
int *generateRandomPatternTop(int patternLength)
{
  int *pattern = new int[patternLength];
  int previousNumber = -1;

  for (int i = 0; i < patternLength; i++)
  {
    int randomNumber;
    do
    {
      randomNumber = random(NUM_LEDS);
    } while (randomNumber == previousNumber);

    pattern[i] = randomNumber;
    previousNumber = randomNumber;
  }

  return pattern;
}

/**
   Function to add a value to the button sequence.

   @param buttonSequence The button sequence array.
   @param value The value to add.
*/
void addToButtonSequence(int buttonSequence[], int buttonSequenceLength, int value)
{
  for (int i = 0; i < buttonSequenceLength - 1; i++)
  {
    buttonSequence[i] = buttonSequence[i + 1];
  }

  buttonSequence[buttonSequenceLength - 1] = value;
}

/**
   Function to compare two arrays.

   @param array1 The first array.
   @param array2 The second array.
   @return True if the arrays are equal, false otherwise.
*/
bool compareArrays(const int *array1, const int *array2, int arrayLength)
{
  for (int i = 0; i < arrayLength; i++)
  {
    if (array1[i] != array2[i])
    {
      return false;
    }
  }
  return true;
}

/**
   Function to run the pattern game.
*/
void runPatternGame()
{

  // Set game level
  int patternLength, patternIncrement, patternSpeed;

  switch (level)
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

  Serial.print("patternLength: ");
  Serial.println(patternLength);
  Serial.print("patternIncrement: ");
  Serial.println(patternIncrement);
  Serial.print("patternSpeed: ");
  Serial.println(patternSpeed);

  // Generate a random pattern
  int* randomPattern = generateRandomPatternTop(patternLength);

  Serial.print("Random pattern::: ");
  for (int i = 0; i < patternLength; i++) {
    Serial.print(randomPattern[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("randomPattern as colors: ");
  for (int o = 0; o < patternLength; o++) {

    switch (randomPattern[o]) {
      case 0:
        Serial.print("red ");
        break;
      case 1:
        Serial.print("green ");
        break;
      case 2:
        Serial.print("blue ");
        break;
      case 3:
        Serial.print("yellow ");
        break;
      default:
        Serial.print("purple ");
        break;
    }
  }
  Serial.println();

  while (!patternGameComplete)
  {

    bool arrayEqual = false; // Flag to check if the buttons pressed array is equal to the required pattern array

    for (int patternSegment = 2; patternSegment < (patternLength + patternIncrement); patternSegment += patternIncrement)
    {

      bool segmentComplete = false;

      while (!segmentComplete)
      {

        Serial.print("Pattern Segment: ");
        Serial.println(patternSegment);

        // Display the colours
        for (int colourIndex = 0; colourIndex < (patternSegment + 1); colourIndex++)
        {
          if (segmentComplete)
          {
            break;
          }

          Serial.print("Colour index: ");
          Serial.println(colourIndex);

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

          setLEDTop(randomPattern[colourIndex], colour);

          if (colourIndex == patternSegment ) {
            fillLights("white");
          }

          for (int lightDelayIncrement = 0; lightDelayIncrement < patternSpeed; lightDelayIncrement++)
          {

            if (segmentComplete)
            {
              break;
            }

            int buttonsPressed[patternLength];
            for (int buttonIdx = 0; buttonIdx < patternLength; buttonIdx++)
            {
              buttonsPressed[buttonIdx] = -1;
            }

            while (true)
            {

              // check if button pressed
              if (checkButtonPressed() > -1)
              {
                // add to queue
                Serial.println("Added to queue.............................................................................");
                updateButtonQueue(buttonsPressed, patternLength);
                Serial.print("Array elements: ");
                for (int y = 0; y < patternLength; y++)
                {
                  Serial.print(buttonsPressed[y]);
                  Serial.print(" ");
                }
                Serial.println();

                // turn off lights
                fillLights("black");

                for (int buttonDelayIncrement = 0; buttonDelayIncrement < BUTTON_PRESS_WAIT_TIME; buttonDelayIncrement++)
                {
                  if (segmentComplete)
                  {
                    break;
                  }

                  if (checkButtonPressed() > -1)
                  {
                    continue;
                  }




                  if (compareArrays(&(buttonsPressed[patternLength - patternSegment]), randomPattern, patternSegment)) { // the subtraction here allows a dynamic start point for the buttons pressed which adds to the end of the queue
                    Serial.println("********************");
                    segmentComplete = true;
                    delay(patternSpeed);
                  }

                  delay(1);
                }
                Serial.println("moving on...");
              }
              else
              {
                break;
              }
            }

            delay(1);
          }
        }
        // wait for next button for a while
        // if no button pressed continue
        // if button not pressed continue
      }
    }
    /*
      for (int j = 0; j < 120; j++) {
        delay(5);
        updateButtonQueue(buttonQueue, patternLength);
        arrayEqual = compareArrays(buttonQueue, randomPattern, patternLength);
      }
      }

      if (arrayEqual) {
      setLEDTop(0, CRGB::Green);
      break;
      }
    */

    // LEDTopWhiteFlash();

    fillLights("green");
    patternGameComplete = true;
  }


  delete[] randomPattern;
}
