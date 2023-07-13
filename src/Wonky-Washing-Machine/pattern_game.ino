#include <FastLED.h>
#include <Servo.h>

#define LEDS_DATA_PIN 6
#define NUM_LEDS 4

#define BUTTON_PIN_RED 30
#define BUTTON_PIN_GREEN 32
#define BUTTON_PIN_BLUE 34
#define BUTTON_PIN_YELLOW 36

/**
 * Flag to track the completion of the pattern game.
 */
bool patternGameComplete = false;

/**
 * Length of the pattern.
 */
int patternLength = 4;

/**
 * Array to store the random pattern.
 */
int* randomPattern = generateRandomPatternTop(patternLength);

/**
 * Array to store the button queue.
 */
int buttonQueue[patternLength];

/**
 * Array to store the LED colours.
 */
CRGB leds[NUM_LEDS];

/**
 * Setup function for the pattern game.
 */
void setupPatternGame() {
  pinMode(BUTTON_PIN_RED, INPUT);
  pinMode(BUTTON_PIN_GREEN, INPUT);
  pinMode(BUTTON_PIN_BLUE, INPUT);
  pinMode(BUTTON_PIN_YELLOW, INPUT);

  FastLED.addLeds<NEOPIXEL, LEDS_DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(10);
}

/**
 * Function to run the pattern game.
 */
void runPatternGame() {
  while (!patternGameComplete) {
    bool arrayEqual = false;

    for (int i = 0; i < patternLength; i++) {
      if (randomPattern[i] >= 0 && randomPattern[i] < NUM_LEDS) {
        unsigned long colour;
        switch (randomPattern[i]) {
          case 0:
            colour = 0xFF0000; // Red
            break;
          case 1:
            colour = 0x00FF00; // Green
            break;
          case 2:
            colour = 0x0000FF; // Blue
            break;
          case 3:
            colour = 0xF6FF00; // Yellow
            break;
          default:
            colour = 0xFFFFFF; // White
            break;
        }
        setLEDTop(randomPattern[i], colour);
      }

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

    // LEDTopWhiteFlash();
  }

  delete[] randomPattern;
}

/**
 * Function to set the colour of a specific LED.
 *
 * @param ledNum The LED number.
 * @param colour The colour to set (in hex format).
 */
void setLEDTop(int ledNum, unsigned long colour) {
  if (ledNum >= 0 && ledNum < NUM_LEDS) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[ledNum] = colour;
    FastLED.show();
  }
}

/**
 * Function to perform a white flash on the LEDs.
 */
void LEDTopWhiteFlash() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();

  delay(1000);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

/**
 * Function to check which button is pressed.
 *
 * @return The button number (0-3) or -1 if no button is pressed.
 */
int checkButtonPressed() {
  if (digitalRead(BUTTON_PIN_RED) == HIGH) {
    return 0;
  } else if (digitalRead(BUTTON_PIN_GREEN) == HIGH) {
    return 1;
  } else if (digitalRead(BUTTON_PIN_BLUE) == HIGH) {
    return 2;
  } else if (digitalRead(BUTTON_PIN_YELLOW) == HIGH) {
    return 3;
  }
  return -1; // No button pressed
}

/**
 * Function to update the button queue.
 *
 * @param buttonQueue The button queue array.
 * @param buttonQueueLength The length of the button queue.
 */
void updateButtonQueue(int buttonQueue[], int buttonQueueLength) {
  int latest = checkButtonPressed();
  if (latest >= 0 && latest != buttonQueue[buttonQueueLength - 1]) {
    addToButtonSequence(buttonQueue, buttonQueueLength, latest);

    switch (latest) {
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
 * Function to generate a random pattern.
 *
 * @param patternLength The length of the pattern.
 * @return An array containing the random pattern.
 */
int* generateRandomPatternTop(int patternLength) {
  int* pattern = new int[patternLength];
  int previousNumber = -1;

  for (int i = 0; i < patternLength; i++) {
    int randomNumber;
    do {
      randomNumber = random(NUM_LEDS);
    } while (randomNumber == previousNumber);

    pattern[i] = randomNumber;
    previousNumber = randomNumber;
  }

  return pattern;
}

/**
 * Function to add a value to the button sequence.
 *
 * @param buttonSequence The button sequence array.
 * @param buttonSequenceLength The length of the button sequence.
 * @param value The value to add.
 */
void addToButtonSequence(int buttonSequence[], int buttonSequenceLength, int value) {
  for (int i = 0; i < buttonSequenceLength - 1; i++) {
    buttonSequence[i] = buttonSequence[i + 1];
  }

  buttonSequence[buttonSequenceLength - 1] = value;
}

/**
 * Function to compare two arrays.
 *
 * @param array1 The first array.
 * @param array2 The second array.
 * @param length The length of the arrays.
 * @return True if the arrays are equal, false otherwise.
 */
bool compareArrays(const int* array1, const int* array2, int length) {
  for (int i = 0; i < length; i++) {
    if (array1[i] != array2[i]) {
      return false;
    }
  }
  return true;
}
