# UniC

**UniC** (Universal Controller) is a wireless controller system designed to work with Windows, Linux, and microcontrollers without requiring platform-specific hardware.

The system consists of a handheld controller and a receiver module communicating over ESP-NOW.

## What it can do

* Emulate a USB gamepad on a PC.
* Emulate a USB mouse and limited keyboard inputs.
* Communicate with external microcontrollers over UART.
* Display latency and packet loss in real time.
* Adjust control parameters using onboard potentiometers.

## Hardware

### Controller

ESP32 DevKit V1 in a 3D-printed enclosure containing:

* 2 joysticks
* 2 switches
* 2 potentiometers
* 8 buttons
* 128×64 OLED display
* Kill switch

The controller generates commands and transmits them to the receiver using ESP-NOW.

### Receiver

ESP32-S3 Mini with two status LEDs.

The receiver handles communication with the controlled system and supports:

* USB HID (gamepad, mouse, keyboard)
* UART

It also measures RTT and monitors connection quality.

## TODO

* [ ] Add functionality for unused buttons.
* [ ] Add R1, R2, L1 and L2 gamepad buttons.
* [ ] Clean up code and use libraries.
