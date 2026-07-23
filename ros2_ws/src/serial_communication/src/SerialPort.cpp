#include "SerialPort.hpp"

SerialPort::SerialPort(){
    SerialPort(stringToCharacterArray("/dev/ttyACM0"), false, false, false, false, false, false, 0, 0, 9600);
}

SerialPort::SerialPort(char* portID){
    SerialPort(portID, false, false, false, false, false, false, 0, 0, 9600);
}

SerialPort::SerialPort(char* portID, int baud){
    SerialPort(portID, false, false, false, false, false, false, 0, 0, baud);
}

SerialPort::SerialPort(char* portID, int minimumData, int timeout, int baud){
    SerialPort(portID, false, false, false, false, false, false, minimumData, timeout, baud);
}

SerialPort::SerialPort(char* portID, bool parityBit, bool extraStopBit, int baud){
    SerialPort(portID, parityBit, extraStopBit, false, false, false, false, 0, 0, baud);
}

SerialPort::SerialPort(char* portID, bool parityBit, bool extraStopBit, int minimumData, int timeout, int baud){
    SerialPort(portID, parityBit, extraStopBit, false, false, false, false, minimumData, timeout, baud);
}

SerialPort::SerialPort(char* portID, bool parityBit, bool extraStopBit, bool hardwareControl, bool controlLines, bool canonical, bool echo, int minimumData, int timeout, int baud)
    : portID(portID), parityBit(parityBit), twoStopBits(extraStopBit), hardwareFlowControlEnabled(hardwareControl),
    enableControlLines(controlLines), canonicalMode(canonical), enableEcho(echo), minimumData(minimumData), timeout(timeout), baudRate(baud){ 

        portIsOpen = false;
    }

SerialPort::SerialPort(std::string portID){
    SerialPort(portID, false, false, false, false, false, false, 0, 0, 9600);
}

SerialPort::SerialPort(std::string portID, int baud){
    SerialPort(portID, false, false, false, false, false, false, 0, 0, baud);
}

SerialPort::SerialPort(std::string portID, int minimumData, int timeout, int baud){
    SerialPort(portID, false, false, false, false, false, false, minimumData, timeout, baud);
}

SerialPort::SerialPort(std::string portID, bool parityBit, bool extraStopBit, int baud){
    SerialPort(portID, parityBit, extraStopBit, false, false, false, false, 0, 0, baud);
}

SerialPort::SerialPort(std::string portID, bool parityBit, bool extraStopBit, int minimumData, int timeout, int baud){
    SerialPort(portID, parityBit, extraStopBit, false, false, false, false, minimumData, timeout, baud);
}

SerialPort::SerialPort(std::string portID, bool parityBit, bool extraStopBit, bool hardwareControl, bool controlLines, bool canonical, bool echo, int minimumData, int timeout, int baud)
    : portID(stringToCharacterArray(portID)), parityBit(parityBit), twoStopBits(extraStopBit), hardwareFlowControlEnabled(hardwareControl),
    enableControlLines(controlLines), canonicalMode(canonical), enableEcho(echo), minimumData(minimumData), timeout(timeout), baudRate(baud){ 

        portIsOpen = false;
    }

SerialPort::~SerialPort(){
    delete portID;
    close(serial_port);
}

void SerialPort::setPortID(char* portID){
    this->portID = portID;
}

void SerialPort::setPortID(std::string portID){
    this->portID = stringToCharacterArray(portID);
}

void SerialPort::toggleParityBit(){
    parityBit = !parityBit;
}

void SerialPort::toggleExtraStopBit(){
    twoStopBits = !twoStopBits;
}

void SerialPort::toggleHardwareControl(){
    hardwareFlowControlEnabled = !hardwareFlowControlEnabled;
}

void SerialPort::toggleControlLines(){
    enableControlLines = !enableControlLines;
}

void SerialPort::toggleCannonicalMode(){
    canonicalMode = !canonicalMode;
}

void SerialPort::toggleEcho(){
    enableEcho = !enableEcho;
}

void SerialPort::toggleReading(){
    enableReading = !enableReading;
    
    if(!enableReading)
        std::cout << '\n' << "CAUTION:" << '\n\n' << "YOU HAVE DISABLED READING FROM THE SERIAL PORT" << '\n\n' << "IF THIS WAS UNINTENTIONAL UNDO THIS" << '\n';
    else
        std::cout << '\n' << "Crisis Averted" << '\n\n' << "Ability to Read Restored :)" << '\n';
}

void SerialPort::setMinimumData(int number){
    minimumData = number;
}

void SerialPort::setTimeout(int number){
    timeout = number;
}

void SerialPort::setBaudRate(int number){
    baudRate = number;
}

void SerialPort::configure(){
    //it is best practice to only modify the bit you are interested in when modifying configuration values, hence the usage of &= and |= below

    // Parity bit
    if(!parityBit)
        tty.c_cflag &= ~PARENB; // No Parity bit
    else
        tty.c_cflag |= PARENB;  // Parrty Bit

    // Stop bits
    if(!twoStopBits)
        tty.c_cflag &= ~CSTOPB; // 1 stop bit
    else
        tty.c_cflag |= CSTOPB;  // 2 stop bits

    // set the number of bits per byte. I have this set to 8 as this set to 8 as that is pretty standard. if you need different,
    // then change twhich line is uncommented below
    tty.c_cflag &= ~CSIZE; // clear all the size bits
    // tty.c_cflag |= CS5; // 5 bits per byte
    // tty.c_cflag |= CS6; // 6 bits per byte
    // tty.c_cflag |= CS7; // 7 bits per byte
    tty.c_cflag |= CS8; // 8 bits per byte

    // Hardware flow control (crtscts)
    // basically, this is for if we've got 2 extra wires that indicate when the device is ready to receive a message
    // it's like adding enable pins to UART communication, in the case of DaVE, we will not need this, but i'll laeve it in for future club members
    if (!hardwareFlowControlEnabled)
        tty.c_cflag &= ~CRTSCTS; // no rts/cts control
    else
        tty.c_cflag |= CRTSCTS;  // hardware flow control enabled

    // enable reading from serial port
    if (enableReading)
        tty.c_cflag |= CREAD;
    else
        tty.c_cflag &= ~CREAD;

    // enable control lines
    if (!enableControlLines)
        tty.c_cflag |= CLOCAL;
    else
        tty.c_cflag &= ~CLOCAL;

    // Disable Canonical Mode
    if (!canonicalMode)
        tty.c_lflag &= ~ICANON;
    else
        tty.c_lflag |= ICANON;

    // enable ECHO
    if (enableEcho){
        tty.c_lflag |= ECHO;   // Enable ECHO
        tty.c_lflag |= ECHOE;  // Enable ECHO erasure
        tty.c_lflag |= ECHONL; // Enable new-line ECHO
    }
    else {
        tty.c_lflag &= ~ECHO;   // disable ECHO
        tty.c_lflag &= ~ECHOE;  // disable ECHO erasure
        tty.c_lflag &= ~ECHONL; // disable new-line ECHO
    }

    // Disable signal chars
    tty.c_lflag &= ~ISIG; // Diable interpretation of INTR, QUIT and SUSP

    // software flow control
    // Realistically, this should always be on for a serial port
    // uncomment next line if you doubt me
    // tty.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off software control

    // Disable Special Handling of Bytes on Receive
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // disables this feature so we just get the raw data

    // output modes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT IN LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT IN LINUX)

    //VMIN and VTIME
    if (minimumData < 0 || minimumData > 255){
        tty.c_cc[VMIN] = 0; // VMIN is represented by one byte, so it is a number between 0 and 255, we will default to 0 if an invalid number is received
    }
    else{
        tty.c_cc[VMIN] = minimumData;
    }

    //NOTE: VTIME is in deciseconds (so a value of 255 for VTIME means that Read() will be blocked for 25.5 seconds)
    if (timeout < 0 || timeout > 255){
        tty.c_cc[VTIME] = 0; // VTIME is represented by one byte, so it is a number between 0 and 255, we will default to 0 if an invalid number is received
    }
    else{
        tty.c_cc[VTIME] = timeout;
    }

    // Baud Rate
    switch(baudRate){
        case 0:      cfsetspeed(&tty, B0);      break;
        case 50:     cfsetspeed(&tty, B50);     break;
        case 75:     cfsetspeed(&tty, B75);     break;
        case 110:    cfsetspeed(&tty, B110);    break;
        case 134:    cfsetspeed(&tty, B134);    break;
        case 150:    cfsetspeed(&tty, B150);    break;
        case 200:    cfsetspeed(&tty, B200);    break;
        case 300:    cfsetspeed(&tty, B300);    break;
        case 600:    cfsetspeed(&tty, B600);    break;
        case 1200:   cfsetspeed(&tty, B1200);   break;
        case 1800:   cfsetspeed(&tty, B1800);   break;
        case 2400:   cfsetspeed(&tty, B2400);   break;
        case 4800:   cfsetspeed(&tty, B4800);   break;
        case 9600:   cfsetspeed(&tty, B9600);   break;
        case 19200:  cfsetspeed(&tty, B19200);  break;
        case 38400:  cfsetspeed(&tty, B38400);  break;
        case 57600:  cfsetspeed(&tty, B57600);  break;
        case 115200: cfsetspeed(&tty, B115200); break;
        case 230400: cfsetspeed(&tty, B230400); break;
        case 460800: cfsetspeed(&tty, B460800); break;
        default:     cfsetspeed(&tty, B9600);
    }

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
}

void SerialPort::begin(){
    // start serial communication
    serial_port = open(portID, O_RDWR);
    portIsOpen = true;

    // check for errors in creating the serial connectiom
    if(serial_port < 0){
        printf("Error %d from open: %s\n", errno, std::strerror(errno));
        return;
    }

    // read in existing settings and handle any error that come up
    if(tcgetattr(serial_port, &tty) != 0){
        printf("Error %d from tcgetattr: %s\n", errno, std::strerror(errno));
    }

    configure();
}

void SerialPort::write(std::string message){
    write(stringToCharacterArray(message));
}

void SerialPort::write(char* message){
    ::write(serial_port, message, sizeof(message));
}

std::string SerialPort::read(){
    int n = ::read(serial_port, &read_buffer, sizeof(read_buffer)); // n is the number of bytes read
    std::string message = "";

    for (int i = 0; i < n; i++){
        message += read_buffer[i];
    }

    return message;
}

char* SerialPort::stringToCharacterArray(std::string message){
    char* newMessage = new char[message.length()];

    // add each letter of the string to the character array
    for(unsigned int i = 0; i < message.length(); i++){
        newMessage[i] = message[i];
    }

    return newMessage;
}

void SerialPort::printConfig(){
    printf("Parity Bit: %s\n", parityBit ? "enabled" : "disabled");
    printf("Second Stop Bit: %s\n", twoStopBits ? "enabled" : "disabled");
    printf("Hardware Flow Control Lines: %s\n", hardwareFlowControlEnabled ? "enabled" : "disabled");
    printf("Reading: %s\n", enableReading ? "enabled" : "disabled");
    printf("Modem-specific control lines: %s\n", enableControlLines ? "enabled" : "disabled");
    printf("Canonical Mode: %s\n", canonicalMode ? "enabled" : "disabled");
    printf("Echo: %s\n", enableEcho ? "enabled" : "disabled");
    printf("Baud Rate: %d\n", baudRate);
}
