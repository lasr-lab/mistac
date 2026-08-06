# Illumination System - Controlling LEDs
Code to set the LEDs of the NeoPixel Ring in the MISTac. The LED color and brightness as well as their illumination type can be set via a python script running on a host PC. There are two illumination types available: static illumination in which the colors stay the same and dynamic illumination in which the LEDs can be set to loop through a cycle. The last LED settings are stored on the MCU and are reloaded whenever the MCU is connected to power, so the LEDs don't have to be re-set everytime the MCU is reconnected to power.

## Flashing c++ code onto the ESP32 MCU
The c++ code is flashed onto the ESP32dev board using [platformio](https://platformio.org/). 

## Setting LEDs
An example for both LED settings is provided in ```example.py```. 

