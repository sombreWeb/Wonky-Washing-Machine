#define POT_PIN_1 A2
#define POT_PIN_2 A3
#define POT_PIN_3 A4

/**
 * @brief Flag indicating whether the knobs process is complete.
 */
bool knobs_complete = false;

/**
 * @brief The desired values for each knob.
 */
int desired_knob1 = 1;
int desired_knob2 = 1;
int desired_knob3 = 6;

void setupKnobs(){
    
  }

/**
 * @brief Runs the knobs process.
 *
 * This function continuously reads the values from three potentiometers and compares them
 * with the desired knob values. If all three knob values match the desired values, the
 * knobs process is considered complete, and the function exits.
 * 
 * The delay at the end stops guessing by requiring the knobs to remain in the correct position
 * for a set number of seconds. 
 */
void runKnobs() {
  while (!knobs_complete) {
    int pot_value1 = mapPotToNumber(analogRead(POT_PIN_1));
    int pot_value2 = mapPotToNumber(analogRead(POT_PIN_2));
    int pot_value3 = mapPotToNumber(analogRead(POT_PIN_3));

    Serial.println("-------------");
    Serial.print("PotValue1: ");
    Serial.println(pot_value1);

    Serial.print("PotValue2: ");
    Serial.println(pot_value2);

    Serial.print("PotValue3: ");
    Serial.println(pot_value3);

    if (pot_value1 == desired_knob1 && pot_value2 == desired_knob2 && pot_value3 == desired_knob3) {
      knobs_complete = true;
    }
    delay(2000);
  }
}

/**
 * @brief Maps the analog input value of a potentiometer to a corresponding number.
 *
 * This function takes an analog input value from a potentiometer and maps it to a specific number
 * based on predefined ranges. The mapping is done to simplify the comparison of potentiometer values.
 *
 * @param value The analog input value from the potentiometer.
 * @return The mapped number corresponding to the potentiometer value.
 */
int mapPotToNumber(int value) {
  if (value >= 0 && value < 205) {
    return 1;
  } else if (value >= 205 && value < 410) {
    return 2;
  } else if (value >= 410 && value < 615) {
    return 3;
  } else if (value >= 615 && value < 820) {
    return 4;
  } else if (value >= 820 && value < 1023) {
    return 5;
  } else {
    return 6;
  }
}
