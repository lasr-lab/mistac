#include <vector>
#include <sstream>

#include <Arduino.h>
#include "pixel_ring.h"

using std::string;
using std::vector;

#define TIMEOUT 5000

PixelRing pixel_ring;
HardwareSerial* serial;

void setup() {
    pixel_ring.begin();
    serial = &pixel_ring.serial.hw_serial;
    serial->println("Setup finished");
}

unsigned long last_received_time=0;
void loop() {
    //delay(1000);
    if (serial->available()) {
        string line = pixel_ring.serial.read_line();
        // read_line returns an empty string for garbage on the line, only a
        // complete command counts as serial communication
        if (!line.empty()) {
            pixel_ring.serial.print_err("taking commands");
            pixel_ring.set_use_stored_settings(false);
            pixel_ring.handle_command(line);
            last_received_time = millis(); // Reset timeout timer
        }
    }
    // If no serial communication for TIMEOUT ms, revert to stored settings
    if (millis()-last_received_time > TIMEOUT && !pixel_ring.get_use_stored_settings()) {
        pixel_ring.serial.print_err("loading_stored");
        pixel_ring.load_stored_led_settings(pixel_ring.get_preferences(),pixel_ring.get_led_map());
        pixel_ring.set_use_stored_settings(true);
    }
  // If dynamic mode is active, the leds are updated, if not nothing happens
  pixel_ring.update_dynamic_leds();
   
}