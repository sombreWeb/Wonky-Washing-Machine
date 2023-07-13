#include <MFRC522.h>

#define RST_PIN 49
#define SS_PIN 53

#define RED_PIN 26
#define GREEN_PIN 24
#define BLUE_PIN 22

/**
 * @brief Flag indicating whether the activation process is complete.
 */
bool activationComplete = false;

/**
 * @brief The desired UID (unique identifier) of the RFID card.
 * @note This is unique to the tag used and must be read and set below.
 */
byte desiredUID[] = {0x2A, 0x4F, 0xF5, 0x81};

/**
 * @brief Instance of the MFRC522 RFID reader.
 */
MFRC522 mfrc522(SS_PIN, RST_PIN);

/**
 * @brief Initializes the activation setup.
 */
void setupActivation() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  mfrc522.PCD_Init();
  
  setActivationLED("red");
}

/**
 * @brief Runs the activation process.
 */
void runActivation() {
  while (!activationComplete) {
    if (checkUIDMatch(desiredUID)) {
      activationComplete = true;
      setActivationLED("green");
    }
  }
}

/**
 * @brief Checks if the detected UID matches the desired UID.
 * @param desiredUID The desired UID to compare against.
 * @return True if the detected UID matches the desired UID, false otherwise.
 */
bool checkUIDMatch(byte* desiredUID) {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] != desiredUID[i]) {
        return false;
      }
    }
    return true;
  }
}

/**
 * @brief Sets the activation LED to the specified colour.
 * @param colour The colour to set the activation LED to ("red", "green", "blue").
 */
void setActivationLED(String colour) {
  if (colour == "red") {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
  } else if (colour == "green") {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
  } else if (colour == "blue") {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, HIGH);
  } else {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
  }
}
