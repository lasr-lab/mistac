#pragma once

#include <Arduino.h>
#include <string>


using std::string;


class SerialCom {

    public: 

        HardwareSerial hw_serial;

        SerialCom(uint8_t uart_nr);


        void begin(unsigned long baudrate);

        int available();


        std::string read_line(unsigned long idle_timeout = 200);

        void print_ack(string msg);

        void print_err(string msg);

};