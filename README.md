ESP32-C3 controlling a 12V DC motor speed and spin direction.

Programmed in C++ with ArduinoIDE a state machine is created to switch from 2 modes: automatic and manual. 
- Automatic: No action is needed. The motor spins clockwise 45s slow, 45s medium speed and 45s fast. Then, it stops and starts rotating counter-clockwise: 45s slow, 45s medium speed and 45s fast.
- Manual: this mode is set when a command is received via bluetooth. There are some recognized commands to set the motor speed and spin direction, which once received, configure the motor for an undefined time period. This mode is unset once an incorrect command is sent or when the client is disconnected.

BTS7960 motor driver is used.
