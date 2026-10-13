#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>

// linux specific headers
#include <fcntl.h>   // contains file controls
#include <errno.h>   // error numbers and descriptions
#include <termios.h> // POSIX serial terminal control definitions
#include <unistd.h>  // Read, write and close functions

/*
    most of this code is not mine, it was taken from a blog (link in the 2026 design guild)
    If you have any wuestions, I encourage you to check it out as the author gives some good expanation about things
    my main contribution was reorganizing it into a class (for reusability purposes)
*/

class SerialPort {
    // Attributes

    struct termios tty;

public:
    int serial_port;        // handle for communicating with the device
    char* portID;           // path to the file associated with our desired serial port i.e "/dev/ttyUSB0"
    char read_buffer [256]; // 256 byte buffer to read into

    bool parityBit;                  // used to select if we want to have a parity bit
    bool twoStopBits;                // used to select if you want 1 stop bit or 2
    bool hardwareFlowControlEnabled; // used to turn on RTS/CTS control (not a feature neede with DaVE)
    bool enableReading;              // used to allow us to read from the serial port
    bool enableControlLines;         // used to set CLOCAL which disables
    bool canonicalMode;              // used to enable cannonical mode (where data is only processed when a newline character is received)
    bool enableEcho;                 // used to select if we want sent bits to be echoed back at us
    bool portIsOpen;                 // used to confirm if the serial port is open

    int minimumData;                 // used to set VMIN, this will change the behaviour of read()
    int timeout;                     // used to set VTIME, this will change the behaviour of read()
    int baudRate;                    // baudrate


    /* NOTE:
        I will not be implementing custom baudrates

        if an invalid baudrate is entered
        (i.e. not in {0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800})
        then it will default to 9600

        if you want custom baud rates, revisit the blog mentioned in the 2026 design guild,
        it has a full walkthrough of it and everything else in this class
    */

    SerialPort();

    SerialPort(char* portID);

    SerialPort(char* portID, int baud);

    SerialPort(char* portID, int minimumData, int timeout, int baud);

    SerialPort(char* portID, bool parityBit, bool extraStopBit, int baud);

    SerialPort(char* portID, bool parityBit, bool extraStopBit, int minimumData, int timeout, int baud);

    SerialPort(char* portID, bool parityBit, bool extraStopBit, bool hardwareControl, bool controlLines, bool canonical, bool echo, int mimimunData, int timeout, int baud);

    SerialPort(std::string portID);

    SerialPort(std::string portID, int baud);

    SerialPort(std::string portID, int minimumData, int timeout, int baud);

    SerialPort(std::string portID, bool parityBit, bool extraStopBit, int baud);

    SerialPort(std::string portID, bool parityBit, bool extraStopBit, int minimumData, int timeout, int baud);

    SerialPort(std::string portID, bool parityBit, bool extraStopBit, bool hardwareControl, bool controlLines, bool canonical, bool echo, int mimimunData, int timeout, int baud);

    ~SerialPort();

    void setPortID(char* portID);

    void setPortID(std::string portID);

    void toggleParityBit();

    void toggleExtraStopBit();

    void toggleHardwareControl();

    void toggleControlLines();

    void toggleCannonicalMode();

    void toggleEcho();

    void toggleReading();

    void setMinimumData(int number);

    void setTimeout(int number);

    void setBaudRate(int number);
private:
    void configure();
public:
    void begin();

    void write(std::string message);

    void write(char* message);

    std::string read();

    static char* stringToCharacterArray(std::string message);

    void printConfig();
};
