
# Thrustmaster T248 Reverse Engineering
![Project Status](https://img.shields.io/badge/status-functional-green)
![Hardware](https://img.shields.io/badge/platform-RP2040-blue)
![Protocol](https://img.shields.io/badge/interface-UART-orange)

---
![My custom wheel](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/blob/main/pictures/My_Wheel.jpg)

A fully reverse-engineered custom steering wheel running on a stock Thrustmaster T248 wheelbase.

This project documents the analysis of the T248 UART protocol, hardware architecture, and firmware behavior, enabling electronic emulation of the original wheel using an RP2040 microcontroller.

⚠️ This project involves hardware modification. Build and use it at your own risk.


---
# Key Features

- Fully compatible with the original T248 wheelbase
- Complete bidirectional UART communication
- RP2040-based wheel emulator (V2.0 stable)
- OLED display for encoder layer visualization
- Interrupt-driven magnetic shift paddles
- Independent from the original wheel PCB

The mechanical base of this wheel is based on the [Turn Vantage GTE DIY Sim Racing Wheel](https://www.printables.com/model/337690-turn-vantage-gte-diy-sim-racing-wheel) from **Turn Racing**. I modified it to suit my needs.

## Project Overview
This project focuses on reverse engineering the **Thrustmaster T248** steering wheel in order to enable fully custom-built steering wheels on a stock T248 wheelbase.

The long-term objective is to fully understand the communication protocol, hardware architecture, and firmware structure well enough to electronically emulate the original wheel.

All detailed technical documentation (protocol captures, PCB reverse engineering, firmware analysis, oscilloscope measurements, etc.) has been moved to the Wiki to keep this README clean and focused.

👉 [**Full Technical Documentation (Wiki)**](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/wiki)

---

## Current Project State

- The T248 UART communication protocol is largely understood.
- The startup handshake and keep-alive mechanism are fully decoded.
- Button and encoder transmission is fully working.
- A **fully functional wheel emulator (V2.0)** has been implemented (without screen data like rpm, gear, lap-time, etc).
- Custom steering wheels can now operate on a stock T248 wheelbase.
- Remaining unknowns are mainly related to:
  - Display data decoding
  - Firmware update structure

---

## Wheel Emulator

The emulator is implemented using:

- **Raspberry Pi Pico (RP2040)**
- Arduino IDE
- UART communication (115200 baud, 8E1)
- Direct electrical connection to the wheelbase

Other 3.3V microcontrollers may be used, but voltage compatibility must be strictly verified. And additional code changes might be necessary.

Development was carried out incrementally through multiple V1.x versions (documented in the Wiki: [4. Wheel Emulator](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/wiki/4.-Wheel-Emulator) ).  
The first stable and practically usable version is V2.0

---

## V2.0 – Stable Wheel Emulator

This version is considered the first fully working implementation suitable for real-world use.

### Features

- Full wheelbase communication (no additional USB cable to PC required)
- Fully working buttons and encoders
- Encoder layer decoding displayed on a small OLED
- Shift paddles handled via hardware interrupts for improved responsiveness
- Optional `DEBUG` flag to disable serial debugging output



### Required Libraries

- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [RPI_PICO_TimerInterrupt (khoih-prog)](https://github.com/khoih-prog/RPI_PICO_TimerInterrupt)
- [Rotary (buxtronix)](https://github.com/buxtronix/arduino/tree/master/libraries/Rotary)
- [arduino-pico core (earlephilhower)](https://github.com/earlephilhower/arduino-pico)

Code:  
👉 [T248_Emulator_V2.0.ino](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/blob/main/Arduino_files_for_RP2040/T248_Emulator_V2.0/T248_Emulator_V2.0.ino)


### Required Components

- Raspberry Pi Pico or Pico 2
- SSD1306 32×128 I²C OLED display
- Rotary encoder (breakout board recommended)
- NPN BJT transistor (e.g., BC548 or similar)
- Push buttons
- Debounce components per button:
  - 10 kΩ pull-up resistor
  - 100 nF capacitor
- Additional resistors for:
  - Transistor base
  - UART TX protection
  - Required pull-ups


### Wiring Diagram

Schematic:

![DIY Wheel Schematic](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/blob/main/pictures/DIY_Wheel_Schematic.png)

📄 [PDF Version](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/blob/main/T248%20DIY%20steering%20wheel%20schematic.pdf)

Notes:

- Every button should include a hardware debounce circuit (except the encoder button, which is handled in software).
- Button mappings are configurable in the source code.
- The unused connector pin corresponds to PA14, likely only required during firmware updates.
- Most encoder breakout boards already include pull-up resistors.



### Connecting to the Wheelbase

1. Remove the original steering wheel (2 screws on the shaft).
2. Carefully pull the wheel off the shaft (requires significant force).
3. Cut the original cable and connect it to your Pico.  
   It is strongly recommended to add a detachable connector for easier servicing.

Example cable colors (may vary):

- Grey → 3.3V  
- Red → PA14 (not required)  
- Orange → RX  
- White → TX  
- Blue → GND  
- Green → RESET  

⚠️ Wire colors may differ between units.  
The safest method is verifying each wire against the steering wheel PCB test pads:

- TP19 / TP5 → 3.3V  
- TP8 → PA14  
- TP6 → RX  
- TP7 → TX  
- TP9 → RESET  

More information:  
👉 [Hardware Analysis (Wiki)](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/wiki/2.--Hardware-Analysis)

---

⚠️ **IMPORTANT WARNING** 

⚠️**Never connect the Pico to USB while it is electrically connected to the wheelbase.**  
The VSYS voltage can exceed 3.3V and permanently damage the wheelbase electronics.

⚠️ This project involves hardware modification.  
You are responsible for any damage to your equipment.

---

## Steering_Wheel_USB (Legacy)

Before the full emulator was completed, a USB-only custom steering wheel implementation was developed.

Characteristics:

- Based on V1.8 logic
- Enumerates as a USB gamepad
- Uses the Arduino Joystick library
- Less optimized interrupt handling
- Still fully functional

This version does **not** communicate with the T248 wheelbase and requires a separate USB connection to the PC.

---

## Open Questions

The following areas still require further investigation:

### Display Data Decoding
- Extraction of telemetry values (RPM, gear, lap time, etc.)
- Purpose of the 12-byte display response frame

### Firmware Update Process
- Does the PC updater flash both wheelbase and wheel firmware?
- Why are two firmware sections stored inside a single `.tmf` file?
- Is wheel firmware transmitted via UART through the wheelbase?

### PA14 Functionality
- Dual-use pin (Mode button / SWCLK)
- Possible boot or flashing role
- Exact behavior during firmware updates remains unclear

---

## Goal

The objective of this project is to:

- Electronically emulate a T248 steering wheel
- Enable fully custom-built steering wheels
- Document the protocol for future development
- Provide a foundation for firmware and display research

This concept is similar to what Taras demonstrated on his [blog](https://rr-m.org/blog/), where he used Arduino code to emulate older Thrustmaster models (T150, TMX, T300, etc.).

---

## Documentation

All detailed research material is available in the Wiki:

- PCB reverse engineering
- Communication protocol breakdown
- Frame analysis
- Encoder decoding
- Firmware structure investigation
- Oscilloscope captures
- And more

👉 [**Project Wiki**](https://github.com/Spb2005/Thrustmaster-T248-reverse-engineering/wiki)
