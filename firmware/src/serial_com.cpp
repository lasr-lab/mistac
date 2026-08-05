//based on digit-probe by Camillo Oeser https://github.com/lasr-lab/digit-probe/tree/main

#include "serial_com.h"
#include <string>
#include <vector>

using std::string;
using std::vector;
#define SERIAL_SIZE_RX  2048 
#define SERIAL_SIZE_TX  2048

SerialCom::SerialCom(uint8_t uart_nr) : hw_serial(uart_nr) { 
    
}


void SerialCom::begin(unsigned long baudrate) {
    // the buffer sizes have to be set before begin(), afterwards the core
    // ignores them and the buffers stay at their default size
    this->hw_serial.setRxBufferSize(SERIAL_SIZE_RX);
    this->hw_serial.setTxBufferSize(SERIAL_SIZE_TX);
    this->hw_serial.begin(baudrate);
    delay(500);
}


int SerialCom::available() {
    return this->hw_serial.available();
}


string SerialCom::read_line(unsigned long idle_timeout) {
    vector<char> buffer;
    unsigned long last_char_time = millis();
    // give up once nothing arrives for idle_timeout ms, so an unterminated
    // fragment can never stall the main loop
    while (millis() - last_char_time < idle_timeout) {
        // make sure a new char can be read in
        if (!this->hw_serial.available()) {
            delay(1); // Small delay to wait for data
            continue;
        }
        char read_char = this->hw_serial.read();
        last_char_time = millis();
        //this->hw_serial.println(read_char);
        switch (read_char) {
            // CR and LF both end a line: other programs on the host (e.g. the
            // ModemManager probe on plug-in) send CR-terminated data, which
            // would otherwise never complete a line
            case '\r':
            case '\n':
                if (buffer.empty()) {
                    break; // skip empty lines and the LF of a CRLF pair
                }
                return string(buffer.begin(), buffer.end());
            default:
                // commands are JSON objects, so drop everything before the
                // opening brace instead of prepending it to a valid command
                if (buffer.empty() && read_char != '{') {
                    break;
                }
                buffer.push_back(read_char);
                break;
        }
    }
    return string(); // nothing usable received, caller discards it
}


void SerialCom::print_ack(string msg) {
    string ack = string("A ") + msg;
    this->hw_serial.println(ack.c_str());
}


void SerialCom::print_err(string msg) {
    string err = string("E ") + msg;
    this->hw_serial.println(err.c_str());
}