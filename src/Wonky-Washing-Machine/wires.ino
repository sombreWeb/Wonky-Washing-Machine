
#include <ArduinoSTL.h>


const int numSensors = 4;          // Number of analog input pins to read
const int sensorPins[] = {A0, A1, A2, A3};  // Analog input pins that sense Vout
int sensorValues[numSensors] = {0};  // SensorPin default values
float Vouts[numSensors] = {0};       // Vout default values
float Vin = 5;                      // Input voltage
float Rref = 20000;                    // Reference resistor's value in ohms

int homePort = 0;
float redPortOffsets[numSensors] = {0, -0.033, -0.025, -0.02};

int numberOfResistanceTests = 100;

struct wireConnection {
  float resistanceValue;
  int redPort;     // Integer value
  char colour;   // Character value
  int blackPort; // Floating-point value
};

std::vector<wireConnection> allWireConnections;

bool compareByColourAndBlackPort(const wireConnection& wire1, const wireConnection& wire2) {
  if (wire1.colour != wire2.colour) {
    return wire1.colour < wire2.colour;
  } else {
    return wire1.blackPort < wire2.blackPort;
  }
}

bool compareByColourAndRedPort(const wireConnection& wire1, const wireConnection& wire2) {
  if (wire1.colour != wire2.colour) {
    return wire1.colour < wire2.colour;
  } else {
    return wire1.redPort < wire2.redPort;
  }
}

double calculatePercentageDifference(double value1, double value2) {

  if (value1 == value2) {
    return 0.0;  // To avoid division by zero
  }

  double percentageDiff = ((value1 - value2) / ((value1 + value2) / 2.0)) * 100.0;
  return percentageDiff;
}

float findAverageResistance(int sensorNumber, int numberOfTests, boolean redOffsetOn = true)
{

  float redOffset = Rref * (1 + redPortOffsets[sensorNumber]);

  float sumResistance = 0;
  for (int j = 0; j < numberOfTests; j++)
  {
    sensorValues[sensorNumber] = analogRead(sensorPins[sensorNumber]);       // Read Vout on analog input pin
    Vouts[sensorNumber] = (Vin * sensorValues[sensorNumber]) / 1023.0;       // Convert Vout to volts
    float resistance = Rref * (1.0 / ((Vin / Vouts[sensorNumber]) - 1.0));   // Calculate tested resistor's value
    sumResistance += resistance;
    delay(5); // Delay to allow the ADC to stabilize between readings
  }
  float averageResistance = sumResistance / numberOfTests; // Calculate average resistance
  if (redOffsetOn) {
    averageResistance = averageResistance * (1 + redPortOffsets[sensorNumber]);
  }

  return averageResistance;
}




std::vector<wireConnection> readCurrentWireSetup(int redOffset, int blackOffset, boolean redOffsetOn = true) {

  char colours[] = {'r', 'g', 'b', 'y'};
  std::vector<wireConnection> currentWireSetup;
  for (int sensorNum = 0; sensorNum < numSensors; sensorNum++) {
    float averageRestistance = findAverageResistance(sensorNum, numberOfResistanceTests, redOffsetOn);
    int colorIndex = (sensorNum - redOffset + numSensors) % numSensors;
    currentWireSetup.push_back({averageRestistance, sensorNum, colours[colorIndex], ((colorIndex + blackOffset) % numSensors)});
  }
  return currentWireSetup;
}

void calibrateRedPorts() {

  std::vector<wireConnection> calibrationValues;

  for (int moveRedWires = 0; moveRedWires < numSensors; moveRedWires++) {
    std::vector<wireConnection> currentValues = readCurrentWireSetup(moveRedWires, 0, false);
    calibrationValues.insert(calibrationValues.end(), currentValues.begin(), currentValues.end());

    do {
      Serial.println("Move each red connection one to the right. Enter ok to continue.");
      while (Serial.available() == 0) {
      }

      String userInput = Serial.readStringUntil('\n');

      if (userInput.equalsIgnoreCase("ok")) {
        Serial.println("Accepted.");
        break;
      } else {
        Serial.println("Invalid input. Please enter 'ok'.");
      }
    } while (true);
  }

  allWireConnections = calibrationValues;

  struct portTotalCount {
    int port;
    double total = 0;
    int count = 0;
  };

  std::vector<portTotalCount> portPercentageAverages(numSensors);

  for (const auto& value : calibrationValues) {

    if (value.redPort != homePort) {
      for (const auto& value2 : calibrationValues) {
        if (value.colour == value2.colour & value2.redPort == homePort) {
          double percentageDiff = calculatePercentageDifference(value.resistanceValue, value2.resistanceValue);
          portPercentageAverages[value.redPort].total += percentageDiff;
          portPercentageAverages[value.redPort].count++;
          portPercentageAverages[value.redPort].port = value.redPort;

        }
      }
    }
  }


  // Calculate and print the average percentage difference for each red port
  for (const auto& entry : portPercentageAverages) {
    int redPort = entry.port;
    Serial.print("Red Port: ");
    Serial.print(redPort);
    Serial.print(", Average Percentage Difference: ");
    double averagePercentageDiff = entry.total / entry.count;
    Serial.println(averagePercentageDiff);

  }

}

void calibrateWires() {

  std::vector<wireConnection> calibrationValues;

  for (int moveBlackWires = 0; moveBlackWires < numSensors; moveBlackWires++) {
    std::vector<wireConnection> currentValues = readCurrentWireSetup(0, moveBlackWires);
    calibrationValues.insert(calibrationValues.end(), currentValues.begin(), currentValues.end());

    do {
      Serial.println("Move each black connection one to the right. Enter ok to continue.");
      while (Serial.available() == 0) {
      }

      String userInput = Serial.readStringUntil('\n');

      if (userInput.equalsIgnoreCase("ok")) {
        Serial.println("Accepted.");
        break;
      } else {
        Serial.println("Invalid input. Please enter 'ok'.");
      }
    } while (true);
  }

  allWireConnections = calibrationValues;
}

void setup()
{
  Serial.begin(9600);   // Initialize serial communications at 9600 bps



  //while (!Serial) {
  //  ; // Wait for the Serial Monitor to open
  //}
}

void loop()
{
  /*
    Serial.print("Port 0: ");
    Serial.println(findAverageResistance(0, 1000));
    Serial.print("Port 1: ");
    Serial.println(findAverageResistance(1, 1000));
    Serial.print("Port 2: ");
    Serial.println(findAverageResistance(2, 1000));
    Serial.print("Port 3: ");
    Serial.println(findAverageResistance(3, 1000));
  */
  calibrateRedPorts();
  //calibrateWires();
  std::sort(allWireConnections.begin(), allWireConnections.end(), compareByColourAndBlackPort);

  for (const auto& connection : allWireConnections) {
    Serial.print("Resistance: ");
    Serial.print(connection.resistanceValue);
    Serial.print(" Ohms, Red Port: ");
    Serial.print(connection.redPort);
    Serial.print(", Colour: ");
    Serial.print(connection.colour);
    Serial.print(", Black Port: ");
    Serial.println(connection.blackPort);
  }
  Serial.println("---------------");

}
