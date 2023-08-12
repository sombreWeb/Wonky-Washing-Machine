#ifndef PATTERN_GAME_H
#define PATTERN_GAME_H

#include <FastLED.h> // FastLED library for controlling LEDs

#define LEDS_DATA_PIN 23      // Data pin for the LED strip
#define NUM_LEDS 4           // Number of LEDs in the strip

#define BUTTON_PIN_RED 31     // Pin for the red button
#define BUTTON_PIN_GREEN 29   // Pin for the green button
#define BUTTON_PIN_BLUE 27   // Pin for the blue button
#define BUTTON_PIN_YELLOW 25 // Pin for the yellow button

extern int patternGameLevel;      // Level for the pattern game
extern bool patternGameComplete;  // Flag to track the completion of the pattern game

extern const int patternLengthLevel1;
extern const int patternLengthLevel2;
extern const int patternLengthLevel3;

extern const int patternIncrementLevel1;
extern const int patternIncrementLevel2;
extern const int patternIncrementLevel3;

extern const int patternSpeedLevel1;
extern const int patternSpeedLevel2;
extern const int patternSpeedLevel3;

extern const unsigned long RED_HEX;
extern const unsigned long GREEN_HEX;
extern const unsigned long BLUE_HEX;
extern const unsigned long YELLOW_HEX;
extern const unsigned long WHITE_HEX;

extern const int BUTTON_PRESS_WAIT_TIME;

extern CRGB leds[NUM_LEDS];

void setupPatternGame();
void setLEDTop(int ledNum, unsigned long colour);
void fillLights(String colour);
int checkButtonPressed();
void updateButtonQueue(int buttonQueue[], int buttonQueueLength);
int *generateRandomPatternTop(int patternLength);
void addToButtonSequence(int buttonSequence[], int buttonSequenceLength, int value);
bool compareArrays(const int *array1, const int *array2, int arrayLength);
void runPatternGame();

#endif
