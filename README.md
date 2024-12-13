# Cortex-M3 Paint Application

This project is a paint application developed for the Cortex-M3 microcontroller. It allows users to draw on the LCD display using various drawing modes and provides an interactive menu for easy navigation.

## Features

- **Drawing Modes**:
  - **Pixel Draw**: Draw individual pixels on the screen.
  - **Line Draw**: Draw lines between two points.
  - **Circle Draw**: Draw circles with a specified radius.
- **Interactive Menu**: Easily switch between different drawing modes and settings.
- **Media Center Integration**: Part of a larger media center project, offering additional functionalities like image viewing and media playback.

## Getting Started

### Prerequisites

- **Hardware**: LPC17xx microcontroller board with an LCD display.
- **Software**: Keil μVision IDE for compiling and uploading the code.

### Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/MahdiHassen/Cortex-M3Paint.git
   ```

2. Open the project in Keil μVision 5 IDE.

3. Compile and upload the code to the microcontroller.

**If the code does not compile in μVision attempt to put all the files into the root directory*

## Usage

- Use the keypad to navigate through the menu.
- Select game to enter M3Paint
- Select the desired drawing mode and draw various shapes, lines and pixels

## Files

The code for M3 Paint mainly lies in the **game.c** file.

## Media Center

The paint application is part of a general media center that includes:

- **Image Viewer**: View and manage images stored on the device.
- **Audio Playback**: Use board as a USB speaker.


