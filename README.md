# UniC

**UniC** (short for **Universal Controller**) is a wireless control system designed to provide plug-and-play compatibility with **Windows**, **Linux**, and **microcontrollers**.

---

## Features

### PC Control Modes

* **Gamepad Mode**

  * Control robots in simulation.
  * Play simple games.

* **Mouse & Keyboard Mode**

  * Remote desktop interaction (Full mouse functionality and limited keyboard support).
    
### Microcontroller Mode

* UART communication with external microcontrollers.
* Add wireless control to existing projects.

## System Architecture

UniC consists of two main modules:

### Controller Unit

The controller is based on an **ESP32 DevKit V1** housed in a custom 3D-printed enclosure.

#### Hardware

* 2 × Joysticks
* 2 × Switches
* 2 × Potentiometers
* 8 × Buttons
* 128×64 OLED Display
* Kill Switch

#### Responsibilities

* Generates control commands.
* Sends commands via ESP-NOW.
* Allows real-time parameter tuning using potentiometers.
* Displays control values, latency, and packet loss.
* Provides an emergency stop.


---

### Receiver / Driver Unit

The receiver is based on an **ESP32-S3 Mini**.

#### Hardware

* ESP32-S3 Mini
* 2 × Status LEDs

#### Responsibilities

* Receives controller commands.
* Measures round-trip latency.
* Displays connection status through LEDs.
* Interfaces with the controlled system.

#### Supported Interfaces

**PC**

* USB HID Gamepad
* USB HID Mouse
* USB HID Keyboard

**Microcontroller**

* UART Communication


