# Wonky-Washing-Machine!

The Wonky Washing Machine is an interactive educational escape room puzzle designed to engage participants in a collaborative repair mission. The project consists of three stages: setting counting knobs, fixing a control panel, and mending the wires. It is being developed using an Arduino Mega 2560 as a core component, adding an exciting tech element to the experience. It includes optional network capabilities through as ESP32 with WebSocket connections. 


## Pins

The following tables show the pin connections for various elements used in the Wonky Washing Machine project. 

## Arduino Mega

| Component             | Pin Name     | Pin Number |
|-----------------------|--------------|------------|
| RFID Reset Pin        | RST_PIN_RFID | 49         |
| RFID Slave Select Pin | SS_PIN_RFID  | 53         |
| Red LED Pin           | RED_PIN      | 48         |
| Green LED Pin         | GREEN_PIN    | 46         |
| Blue LED Pin          | BLUE_PIN     | 47         |
| Potentiometer 1       | POT_PIN_1    | A3         |
| Potentiometer 2       | POT_PIN_2    | A2         |
| Potentiometer 3       | POT_PIN_3    | A1         |
| LED Data Pin          | LEDS_DATA_PIN| 23         |
| Number of LEDs        | NUM_LEDS     | 4          |
| Red Button Pin        | BUTTON_PIN_RED    | 31   |
| Green Button Pin      | BUTTON_PIN_GREEN  | 29   |
| Blue Button Pin       | BUTTON_PIN_BLUE   | 27   |
| Yellow Button Pin     | BUTTON_PIN_YELLOW | 25   |
| Relay Pin             | RELAY_PIN   | 2          |
| Top Servo Pin         | TOP_SERVO   | 3          |
| Side Servo Pin        | SIDE_SERVO  | 4          |
| Bottom Servo Pin      | BOTTOM_SERVO| 5          |
| Port Pin 0            | PORT_PIN_0  | 0          |
| Port Pin 1            | PORT_PIN_1  | 1          |
| Port Pin 2            | PORT_PIN_2  | 2          |
| Port Pin 3            | PORT_PIN_3  | 3          |
| Matrix Slave Select Pin| SS_PIN_MATRIX| 19        |
| Serial RX (UART2 RXD2) | RXD2     | 16         |
| Serial TX (UART2 TXD2) | TXD2     | 17         |
| SDA (I2C)     | SDA      | 20         |
| SCL (I2C)     | SCL      | 21         |
| MISO (SPI)    | MISO     | 50         |
| MOSI (SPI)    | MOSI     | 51         |
| SCK (SPI)     | SCK      | 52         |

NOTE: The SPI connections are bussed. 

## ESP-32

| Component              | Pin Name | Pin Number |
|------------------------|----------|------------|
| Serial RX (UART2 RXD2) | RXD2     | 16         |
| Serial TX (UART2 TXD2) | TXD2     | 17         |





