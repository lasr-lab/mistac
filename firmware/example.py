import json
from python_src.pixel_ring import PixelRing

MCU_COM_BAUDRATE = 115200
MCU_COM_TIMEOUT = 2

# connect to pixel ring
com_target = PixelRing.find_mcu()
print(f"Found MCU COM port with name: {com_target}\n")
pixel_ring = PixelRing(mcu_com_port=com_target,
                            mcu_com_baudrate=MCU_COM_BAUDRATE,
                            mcu_com_timeout=MCU_COM_TIMEOUT)

#test dynamic or staic led setting
print("dynamic_leds? [Y / n]")
cont = input("> ")
if cont.capitalize().startswith('Y'):
    print('setting dynamic leds')
    #set dynamic LEDs
    #set the id of the LEDs which will be used in each cycle
    ids = [[1,2,3],[3,4,5],[1,2,3]]
    #set the color brightness values [r,g,b] for each LED id and each cycle
    colors = [[[100,0,0],[0,100,0],[0,0,100]],
                [[0,100,0],[255,255,255],[0,100,100]],
                [[0,90,0],[0,0,80],[100,0,0]]
                ]
    #set wait time per cycle in ms
    wait_times = [1000,1000,1000]

    pixel_ring.set_leds_dynamic(ids,colors, wait_times)
else:
    print('setting static leds')
    #set static LEDs
    #set lED ids
    ids = [1,2,3]
    #set corresponding color brightness values
    colors = [[255,0,0],[0,255,0],[0,0,255]]
    pixel_ring.set_leds_static(ids,colors)

#test if command was successfully loaded onto ESP32
print("test_loading? [Y / n]")
cont = input("> ")
if cont.capitalize().startswith('Y'):
    load_data = {
        "command": "load_stored"
    }
    json_string_load = json.dumps(load_data)
    pixel_ring.mcu_com.printline(json_string_load)

#turn off pixel ring
pixel_ring.poweroff()