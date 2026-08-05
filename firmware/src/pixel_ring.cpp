#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "pixel_ring.h"


using std::string;
using std::istringstream;
using std::vector;
using std::array;


PixelRing::PixelRing():
    serial(HARDWARE_SERIAL_NUM),
    pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800)
    {}


void PixelRing::begin() {
    if (!this->serial.hw_serial) {
        this->serial.begin(BAUDRATE);
    }
    //start the pixels
    pixels.begin();
    pixels.clear();

}


void PixelRing::handle_command(string line) {
    HardwareSerial* hw_serial = &this->serial.hw_serial;

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, line);
    led_map.clear();
    if (error) {
        hw_serial->println("Failed to parse JSON!");
        return;
    }

    string cmd = doc["command"].as<string>();
    hw_serial->println("Received Command");
    if (cmd == "set_static") {
        JsonArray ids = doc["id"];
        JsonArray colors = doc["color"];

        //disable dynamic mode:
        dynamic_led_state.active = false;

        if (ids.size() != colors.size()) {
            hw_serial->println("Mismatched LED IDs and colors!");
            return;
        }

        for (size_t i = 0; i < ids.size(); i++) {
            int led = ids[i];
            JsonArray color = colors[i];
            //print for debugging
            
            /*
            hw_serial->println("LED ID:");
            hw_serial->println(led);
            hw_serial->println("colors:");
            hw_serial->println(int(color[0]));
            hw_serial->println(int(color[1]));
            hw_serial->println(int(color[2]));
            */
            
            if (color.size() == 3) { // Ensure RGB array is valid
                led_map[led] = {color[0], color[1], color[2]};
            }
        }

        // Set and store the LEDs
        set_static_leds(led_map);   
        store_led_settings(preferences, led_map);
    }
    
    if (cmd == "set_dynamic") {
        JsonArray cycle = doc["cycle"];
        bool repeat = doc["repeat"];

        // store LED IDs for each cycle step
        vector<vector<int>> led_ids; 
        // store RGB colors for each cycle step
        vector<vector<array<int, 3>>> colors;
        // store wait times for each step
        vector<int> wait_times; 


        // clear dynamic_led_state struct

        dynamic_led_state.led_ids.clear();
        dynamic_led_state.colors.clear();
        dynamic_led_state.wait_times.clear();

        // loop over cycle array to retrieve the individual settings 
        // for each step in the cycle
        for (JsonObject step : cycle) {
            JsonArray ids = step["id"];
            JsonArray colorArr = step["color"];
            int wait_time = step["wait_time"];

            if (ids.size() != colorArr.size()) {
                hw_serial->println("Mismatched IDs and colors!");
                return;
            }

            vector<int> step_ids;
            vector<array<int, 3>> step_colors;

            for (size_t i = 0; i < ids.size(); i++) {
                int led = ids[i];
                JsonArray color = colorArr[i];
                //hw_serial->println(led);
                if (color.size() == 3) { // Ensure valid RGB format
                    step_ids.push_back(led);
                    step_colors.push_back({color[0], color[1], color[2]});
                    
                }
            }

            // insert ids and colors of current step into dynamic_led_state struct
            dynamic_led_state.led_ids.push_back(step_ids);
            dynamic_led_state.colors.push_back(step_colors);
            dynamic_led_state.wait_times.push_back(wait_time);

        }
        // set repeat variable in dynamic_led_state
        dynamic_led_state.repeat = repeat;
        // set current step to 0
        dynamic_led_state.current_step = 0;
        // set dynamic mode true
        //dynamic_mode = true;  
        dynamic_led_state.active = true;
        // store settings
        store_led_settings(preferences);
        //no need to call a function because update_dynamic_leds() is checked in the main loop
    }
    // for debugging --> allows to check if loading function works 
    // while serial is connected
    if (cmd == "load_stored"){
        load_stored_led_settings(preferences, led_map);
    }

}

void PixelRing::set_static_leds(const std::map<int,array<int,3>>& led_input_map){
    HardwareSerial* hw_serial = &this->serial.hw_serial;
    pixels.clear();
    for (const auto& pair : led_map){
        //pair.first contains the led id
        //pair.second[0] the r value, pair.second[1] the g value, pair.second[2] the b value
        pixels.setPixelColor(pair.first, pixels.Color(pair.second[0],pair.second[1], pair.second[2]));
        hw_serial->println("inside set_static_leds");
        hw_serial->println(pair.first);
        hw_serial->println(pair.second[0]);
    }
    pixels.show();

}

void PixelRing::update_dynamic_leds() {

    // if dynamic mode is not activate don't do anything
    if (!dynamic_led_state.active || dynamic_led_state.led_ids.empty()) return;

    unsigned long now = millis();
    if (now - dynamic_led_state.last_update >= dynamic_led_state.wait_times[dynamic_led_state.current_step]) {
        // Update LEDs for the current step
        //clear led_map
        led_map.clear();
        for (size_t i = 0; i < dynamic_led_state.led_ids[dynamic_led_state.current_step].size(); i++) {
            int led = dynamic_led_state.led_ids[dynamic_led_state.current_step][i];
            led_map[led] = dynamic_led_state.colors[dynamic_led_state.current_step][i];
        }

        set_static_leds(led_map);  // Apply changes

        // Move to the next step in the cycle
        dynamic_led_state.current_step++;
        if (dynamic_led_state.current_step >= dynamic_led_state.led_ids.size()) {
            if (dynamic_led_state.repeat) {
                dynamic_led_state.current_step = 0;
            } else {
                dynamic_led_state.active = false;  // Stop if not repeating
            }
        }

        dynamic_led_state.last_update = now;  // Reset timer
    }
}

void PixelRing::load_stored_led_settings(Preferences &preferences, std::map<int, std::array<int, 3>> &led_map) {
    HardwareSerial* hw_serial = &this->serial.hw_serial;
    
    preferences.begin("led_prefs", true);
    String jsonString = preferences.getString("led_map", "{}");
    preferences.end();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonString.c_str()); 
    
    if (error) {
        hw_serial->print("Failed to load stored settings!");
        return;
    }
    if (doc["dynamic"]){
        JsonArray ids = doc["id"];
        JsonArray colors = doc["color"];
        JsonArray waitTimesArray = doc["wait_times"];

        // start from a clean cycle, otherwise every load appends another copy
        // of the stored steps to the ones already in dynamic_led_state
        dynamic_led_state.led_ids.clear();
        dynamic_led_state.colors.clear();
        dynamic_led_state.wait_times.clear();
        dynamic_led_state.current_step = 0;

        // loop through nested ids
        for (size_t i = 0; i < ids.size(); i++) {
            //create vectors for the values of the individual steps
            // (per step, so a step doesn't inherit the previous steps' leds)
            vector<int> step_ids;
            vector<array<int, 3>> step_colors;
            hw_serial->println("inside first loop!");
            JsonArray s_ids = ids[i];
            JsonArray s_colors = colors[i];
            int wait_time = waitTimesArray[i];

            for (size_t j =0; j<s_ids.size(); j++){
            
                int led = s_ids[j];
                step_ids.push_back(led);
                step_colors.push_back({int(s_colors[j][0]), int(s_colors[j][1]), int(s_colors[j][2])});
                // uncomment for debugging:
                //hw_serial->println(led);
                //hw_serial->println(int(s_colors[j][0]));

            }
            // insert ids and colors of current step into dynamic_led_state struct
            dynamic_led_state.led_ids.push_back(step_ids);
            dynamic_led_state.colors.push_back(step_colors);
            dynamic_led_state.wait_times.push_back(wait_time);
       

        }
        dynamic_led_state.active = true;
        dynamic_led_state.repeat = doc["repeat"];

        // no need to call dynamic update as is handled in main
    }
    else{
        JsonArray ids = doc["id"];
        JsonArray colors = doc["color"];

        // stored settings are static, a still active cycle would overwrite them
        dynamic_led_state.active = false;

        //clear led_map
        led_map.clear();
        for (size_t i = 0; i < ids.size(); i++) {
            int led = ids[i];
            JsonArray color = colors[i];

            if (color.size() == 3) {
                led_map[led] = {color[0], color[1], color[2]};
            }
        }
        // set static leds
        set_static_leds(led_map);
    }
}   

void PixelRing::store_led_settings(Preferences &preferences) {
    //create json file
    DynamicJsonDocument doc(1024);
    doc["dynamic"] = true;
    JsonArray idArray = doc["id"].to<JsonArray>();
    JsonArray colorArray = doc["color"].to<JsonArray>();
    JsonArray waitTimesArray = doc["wait_times"].to<JsonArray>();
    doc["repeat"] = dynamic_led_state.repeat;
    
    if (!dynamic_led_state.active){
        return;
    }
    // take entries from dynamic_led_state
    //add led ids
    for (size_t i = 0; i<dynamic_led_state.led_ids.size(); i ++){
        
        JsonArray step_id = idArray.add<JsonArray>();
        for (size_t j = 0; j<dynamic_led_state.led_ids[i].size(); j++){
            step_id.add(dynamic_led_state.led_ids[i][j]);
            
        }
    }

    // add colors
    for (size_t i=0; i<dynamic_led_state.colors.size(); i++){
        JsonArray step_colorArray = colorArray.add<JsonArray>();
        for (int j = 0; j < dynamic_led_state.colors[i].size(); j++) {
            JsonArray color = step_colorArray.add<JsonArray>();
            color.add(dynamic_led_state.colors[i][j][0]); // Red
            color.add(dynamic_led_state.colors[i][j][1]); // Green
            color.add(dynamic_led_state.colors[i][j][2]); // Blue
        }
    }
    // add waittimes
    for (size_t i=0; i<dynamic_led_state.wait_times.size(); i++){
        waitTimesArray.add(dynamic_led_state.wait_times[i]);

    }

    //serialize the entries made
    String jsonString;
    serializeJson(doc, jsonString);

    preferences.begin("led_prefs", false);
    preferences.putString("led_map", jsonString);
    preferences.end();
}


void PixelRing::store_led_settings(Preferences &preferences, const std::map<int, std::array<int, 3>> &led_map) {
    //create json file
    DynamicJsonDocument doc(1024);
    doc["dynamic"] = false;
    JsonArray idArray = doc["id"].to<JsonArray>();
    JsonArray colorArray = doc["color"].to<JsonArray>();

    // loop over led_map and store color and led id into JSon Array
    for (const auto& entry : led_map) {
        idArray.add(entry.first); // Store LED index
        JsonArray color = colorArray.add<JsonArray>();
        color.add(entry.second[0]); // Red
        color.add(entry.second[1]); // Green
        color.add(entry.second[2]); // Blue
    }
    //serialize the entries made
    String jsonString;
    serializeJson(doc, jsonString);

    preferences.begin("led_prefs", false);
    preferences.putString("led_map", jsonString);
    preferences.end();
}

Preferences& PixelRing::get_preferences(){
    return preferences;

}

std::map<int, array<int,3>>& PixelRing::get_led_map(){
    return led_map;
}

bool PixelRing::get_use_stored_settings(){
    return use_stored_settings;
}

void PixelRing::set_use_stored_settings( bool use_stored){
    use_stored_settings = use_stored;

}