#include <Wire.h>
#include <string>
#include <vector>
#include <array>
#include <map>

#include <unistd.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "serial_com.h"

using std::array;




#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN        14 //input pin number
#define NUMPIXELS 8 //ring size

#define HARDWARE_SERIAL_NUM 0 
#define BAUDRATE 115200

struct DynamicLedState {
    std::vector<std::vector<int>> led_ids;
    std::vector<std::vector<array<int, 3>>> colors;
    std::vector<int> wait_times;
    bool repeat = false;
    size_t current_step = 0;
    unsigned long last_update = 0;
    bool active = false;
};

class PixelRing {
    
    
    //public:
    private:
    std::map<int, array<int,3>> led_map; //doesn't work with fixed-size arrays
    Adafruit_NeoPixel pixels;
    Preferences preferences;

    bool use_stored_settings = false;
    DynamicLedState dynamic_led_state;

    public: 


        SerialCom serial; 


        PixelRing();

        void begin();

        void handle_command(string line);

        void set_static_leds(const std::map<int,array<int,3>>& led_input_map);
        void update_dynamic_leds();
        void load_stored_led_settings(Preferences &preferences, std::map<int, std::array<int, 3>> &led_map);

        void store_led_settings(Preferences &preferences, const std::map<int, std::array<int, 3>> &led_map);
        void store_led_settings(Preferences &preferences);

        Preferences& get_preferences ();
        std::map<int, array<int,3>>& get_led_map ();

        bool get_use_stored_settings ();
        void set_use_stored_settings( bool use_stored);



};