## Arduino Alarm Clock
A sophisticated embedded alarm clock system built with Arduino featuring dual displays, EEPROM persistence, and event-driven architecture.
The project demonstrates advanced embedded systems concepts including state machines, non-blocking timing, and peripheral interfacing.

## Features
- Dual Display System: 16x2 LCD for time + 128x32 OLED for status messages

- Real-time Clock: Software-based timekeeping with hours, minutes, and seconds

- Configurable Alarm: Set custom alarm times with persistent EEPROM storage

- Musical Alarm: Plays "Take On Me" melody through buzzer with non-blocking playback

- State Machine: Four operational modes (Normal, Set Hour, Set Minute, Alarm Triggered)

- Event-Driven Design: Fully asynchronous with no blocking calls

- Smart Persistence: EEPROM storage with magic number validation and write-on-change

- Dual Button Control: Intuitive interface for alarm setting and control

## Hardware Components
- Arduino Uno/Nano

- 16x2 LCD Display

- 128x32 OLED Display (I2C)

- Piezo Buzzer

- 2x Push Buttons

- Breadboard and Wires

## Wiring Guide
LCD Connections:
- LCD RS pin → digital pin 12
- LCD Enable pin → digital pin 11
- LCD D4 pin → digital pin 5
- LCD D5 pin → digital pin 4
- LCD D6 pin → digital pin 3
- LCD D7 pin → digital pin 2
- LCD R/W pin → GND
- LCD VSS pin → GND
- LCD VDD pin → 5V
- LCD A pin → 5V (backlight)
- LCD K pin → GND
  
Additional Components:
- OLED SDA → A4 | OLED SCL → A5 | OLED VCC → 5V | OLED GND → GND
- Buzzer → digital pin 8
- Set Button → digital pin 6 → GND
- Stop Button → digital pin 7 → GND

## Button Controls:

SET: Enters setup mode, increments values, navigates menus
STOP: Confirms selections, stops alarm, exits modes

## Alarm System:
- Triggers precisely at the set time
- Plays looping musical melody
- Provides visual feedback on both displays
- Can be silenced with STOP button
