import serial
import struct
import threading
import typing


class MCUcom:

    def __init__(self, port: str | None, baudrate: int, timeout: float | None=None) -> None:
        self._mcu_serial = serial.Serial(port, baudrate, timeout=timeout)

    
    def open(self) -> None:
        self._mcu_serial.open()

    
    def is_open(self) -> bool:
        return self.is_open

    
    def close(self) -> None:
        self._mcu_serial.close()


    def is_open(self) -> bool:
        return self._mcu_serial.is_open


    def available(self) -> int:
        return self._mcu_serial.in_waiting


    def read(self, size: int = 1) -> bytes:
        received = self._mcu_serial.read(size)
        if len(received) < size:
            raise TimeoutError(f"Tried to read {size} bytes, but only received {len(received)} bytes containing {received}")
        return received


    def read_all(self) -> bytes | None:
        return self._mcu_serial.read_all()


    def readline(self) -> bytes:
        response = self._mcu_serial.readline()
        if len(response) == 0:
            raise TimeoutError("Did not get response from ESP32 in time")
        if not chr(response[-1]) == '\n':
            raise TimeoutError(f"Did not read full line from ESP32 in time. Response was: {response}")
        return response


    def readline_monitor(self) -> None:     # TODO add threading.Event
        while True:
            if self.available():
                response = self.readline()
                print(response);


    def printline(self, string: str) -> None:
        print(string)
        try:
            b_written =self._mcu_serial.write((string+'\n').encode('utf-8'))
            print(f'wrote {b_written} bytes')
        except Exception as e:
            print(e)
        
        self._mcu_serial.flush()




if __name__ == "__main__":
    mcu_com = MCUcom()

    mcu_monitor = threading.Thread(target=mcu_com.readline_monitor)
    mcu_monitor.start();
    
    while True:
        print('com is running')
        inp = input()
        if inp[0].upper() in ['S', 'D']:
            mcu_com.printline(inp)
        else:
            print(f"{inp[0]} not recognized, command hasn't been sent to ESP32")