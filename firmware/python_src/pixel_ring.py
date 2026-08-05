from datetime import datetime
from time import sleep
import typing
import threading
import os
import serial
import serial.tools.list_ports
import json

from python_src.mcu_com import MCUcom


class PixelRing:

    def __init__(self,
                 mcu_com_port,
                 mcu_com_baudrate=115200,
                 mcu_com_timeout=2):
        
        self.mcu_com_baudrate = mcu_com_baudrate
        self.mcu_com_timeout = mcu_com_timeout
        self.mcu_com = MCUcom(mcu_com_port, mcu_com_baudrate, mcu_com_timeout)
        if not self.mcu_com.is_open():
            self.mcu_com.open()
        self.mcu_com.read_all()
                


    @staticmethod
    def find_mcu():
        print("To automatically recognize the ESP32 Dev Module that is connected to the pixel ring: Unplug the ESP32 Dev Module from your Computer, type <m> and press <Enter>")
        print("Or keep ESP32 Dev Module attached, if you are sure it provides the only COM port in your system: Press <ENTER>")
        find_device = input("> ")
        if find_device in ["<m>", 'm']:
            if find_device == "<m>":
                print("Hint: you don't need to type < and >; m suffices")
            list_without_target = serial.tools.list_ports.comports()
            print("Please plug the ESP32 Dev Module that is attached to the pixel ring into your PC and press <ENTER>")
            input("> ")
            list_with_target = serial.tools.list_ports.comports()
            target = set(list_with_target).difference(set(list_without_target))
            if len(target) == 0:
                print("Could not find a target COM port, please try again")
                exit(1)
            if len(target) > 1:
                print("Too many COM ports appeared when plugging in ESP32 Dev Module, please try again")
                exit(1)
        else:
            target = serial.tools.list_ports.comports()
            target = list(filter(lambda t: t.device.startswith("/dev/ttyUSB"), target))
            if len(target) == 0:
                print("Could not find a target COM port, please try again")
                exit(1)
            if len(target) > 1:
                print("Too many COM ports are attached to the system, please try again, possibly entering <m>")
                exit(1)
        target = target.pop().device
        return target


    def poweroff(self):
        '''Close serial connection'''
        self.mcu_com.close()

    def set_leds_static(self, led_ids, colors):
        '''Set static colors for LEDs 
        
        Keyword arguments:
        led_ids: list of ids of the LEDS [0,7]
        colors: nested list with rgb values belonging to ids
        '''
        
        # create data for JSon object
        data =  {
                "command": "set_static",
                "id": led_ids,
                "color": colors
                }
        # create json_string
        json_string = json.dumps(data)
        # send string to esp32 via serial
        self.mcu_com.printline(json_string)

    def set_leds_dynamic(self, led_ids, colors, wait_times, repeat = True):
        '''Set dynamic color cycle for LEDs 
        
        Keyword arguments:
        led_ids (list of lists): Each sublist contains LED ids [0,7] for that step.
        colors (list of lists): rgb values belonging to LED ids
        wait_times (list of int): Wait times for each step
        repeat (bool): Whether the cycle should repeat
        '''
        
        cycle = []
        # create the list of the individual steps in the cycle
        for i in range(len(led_ids)):
            step = {
                "id": led_ids[i],
                "color": colors[i],
                "wait_time": wait_times[i]
                }
            cycle.append(step)
        
        # create data for JSon object
        data = {
            "command": "set_dynamic",
            "cycle": cycle,
            "repeat": repeat
            }
        # create json_string
        json_string = json.dumps(data)
        # send string to esp32 via serial
        self.mcu_com.printline(json_string)

