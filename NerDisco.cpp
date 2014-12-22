#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "NerDisco.h"
#include "consolestyle.h"

//---------------------------------------------------------------------------------------------------------------------------

const char magicString[3] = {'A', 'd', 'a'};
std::vector<uint8_t> displayData; //string containing the whole command

std::string serialPortName = ""; //!<Default serial port device name.
bool beVerbose = false; //!<Set to true to display more output.

bool autodetectPort = false; //!<Set to true to autodetect the port upon start.
const std::string possiblePortNames[] {
    "USB0", "USB1", "USB2", "USB3", "USB4", "USB5", "USB6", "USB7", "USB8", "USB9", 
    "ACM0", "ACM1", "ACM2", "ACM3", "ACM4", "ACM5", "ACM6", "ACM7", "ACM8", "ACM9", ""
}; //!<List of possible path name. Simplest method I guess...

//---------------------------------------------------------------------------------------------------------------------------

void printVersion()
{
	std::cout << ConsoleStyle(ConsoleStyle::GREEN) << "NerDisco " << MAMEDUINO_VERSION_STRING << ConsoleStyle() << " - The JACKaudio boblight client" << std::endl;
}

void printUsage()
{
    std::cout << "Usage:" << ConsoleStyle(ConsoleStyle::CYAN) << " nerdisco <SERIAL_DEVICE> <COMMAND>" << ConsoleStyle() << std::endl;
    std::cout << "SERIAL_DEVICE should be e.g. " << ConsoleStyle(ConsoleStyle::CYAN) << "/dev/ttyACM0" << ConsoleStyle() << 
                 ", or use " << ConsoleStyle(ConsoleStyle::CYAN) << "-a" << ConsoleStyle() << " to auto-detect it." << std::endl;    
    std::cout << "Valid commands:" << std::endl;
    std::cout << ConsoleStyle(ConsoleStyle::CYAN) << "-h/-?/--help" << ConsoleStyle() << " - Show this help." << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << ConsoleStyle(ConsoleStyle::CYAN) << "nerdisco -a -r on" << ConsoleStyle() << " (auto-detect serial port, turn coin rejection on)" << std::endl;
}

bool readArguments(int argc, const char * argv[])
{
    //first argument must be device, autodetect or help command
    std::string argument = argv[1];
    if (argument.length() > 5 && argument.substr(0, 5) == "/dev/") {
        //serial port passed on command line
        serialPortName = argument;
    }
    else if (argument == "-a") {
        //autodetect command passed
        autodetectPort = true;
    }
    else if (argument == "-?" || argument == "-h" || argument == "--help") {
        //help command passed
        printUsage();
        exit(0);
    }
    else {
        std::cout << ConsoleStyle(ConsoleStyle::RED) << "Error: First argument must be a serial port device string." << ConsoleStyle() << std::endl;
        return false;
    }
    for(int i = 2; i < argc;) {
        //read argument from list
        argument = argv[i++];
        //check what it is
        if (argument == "-d") {
            /command = BLA;
			return true;
        }
        else {
            std::cout << ConsoleStyle(ConsoleStyle::RED) << "Error: Unknown command \"" << argument << "\"!" << ConsoleStyle() << std::endl;
            return false;
        }
    }
    return false;
}

bool serialPortExists(const std::string & portName)
{
    int portHandle = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (portHandle <= 0) {
        return false;
    }
    close(portHandle);
    return true;
}

bool openSerialPort(int & portHandle, const std::string & portName, termios * oldOptions)
{
    //try opening serial port
    if (beVerbose) {
        std::cout << "Opening serial port " << portName << " ..." << std::endl;
    }
    portHandle = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (portHandle <= 0) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Error: Failed to open serial port " << portName << "!" << ConsoleStyle() << std::endl;
		return false;
	}
	//no set serial port to proper settings
	if (beVerbose) {
    	std::cout << "Setting serial port to 115200bps, 8N1 mode..." << std::endl;
    }
	//store current terminal options
	tcgetattr(portHandle, oldOptions);
	//clear new terminal options
	termios options;
	memset(&options, 0, sizeof(termios));
	//set baud rate to 38400 Baud
	cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    //set mode to 8N1
    options.c_cflag |= (CLOCAL | CREAD); //Enable the receiver and set local mode
    options.c_cflag &= ~PARENB; //no parity
    options.c_cflag &= ~CSTOPB; //one stop bit
    options.c_cflag &= ~CSIZE; //size mask flag
    options.c_cflag |= CS8; //8 bit
    //set raw output
    options.c_oflag &= ~OPOST;
    //set input mode (non-canonical, no echo)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //turn parity off
    //options.c_iflag = IGNPAR;
    //flush serial port
    //tcflush(serialPort, TCIFLUSH);
	//set terminal options
	if (tcsetattr(portHandle, TCSANOW, &options) != 0) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Error: Failed set serial port options!" << ConsoleStyle() << std::endl;
		close(portHandle);
		return false;
	}
	return true;
}

void closeSerialPort(const int portHandle, const termios * oldOptions)
{
	//restore old port settings
	tcsetattr(portHandle, TCSAFLUSH, oldOptions);
	//close port and terminate
	close(portHandle);
}

bool writeToSerialPort(const int portHandle, const unsigned char * data, const ssize_t size)
{
	//write bytes to the port
	ssize_t n = write(portHandle, data, size);
	if (n != size) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Error: Failed write to serial port!" << ConsoleStyle() << std::endl;
		return false;
	}
	return true;
}

int main(int argc, const char * argv[])
{
	setup();

    printVersion();
    
    if (argc < 2 || !readArguments(argc, argv) || command == BAD_COMMAND) {
        std::cout << std::endl;
        printUsage();
        return -1;
    }

    termios oldOptions;
    int portHandle = -1;
    if (autodetectPort) {
        //try to autodetect port. get post names from list
        std::string portNameCandidate;
        for (int i = 0; !possiblePortNames[i].empty() && serialPortName.empty(); ++i) {
            //try to open port
            portNameCandidate = "/dev/tty" + possiblePortNames[i];
            if (serialPortExists(portNameCandidate) && openSerialPort(portHandle, portNameCandidate, &oldOptions)) {
                //opening worked. send version command
                std::vector<uint8_t> versionCommand;
                //versionCommand.push_back('?');
                //versionCommand.push_back(COMMAND_TERMINATOR);
                if (writeToSerialPort(portHandle, versionCommand.data(), versionCommand.size() * sizeof(uint8_t))) {
                    //sending worked. receive response.                           
                    std::string response;
                    if (getResponseFromSerial(portHandle, response) && response.substr(0, 10) == "MAMEduino ") {
                        if (beVerbose) {
                            std::cout << ConsoleStyle(ConsoleStyle::GREEN) << response << " found at " << portNameCandidate << "." << ConsoleStyle() << std::endl;
                        }
		                serialPortName = portNameCandidate;
                    }
                }
                closeSerialPort(portHandle, &oldOptions);
            }
        }
        if (serialPortName.empty()) {
            std::cout << ConsoleStyle(ConsoleStyle::RED) << "Error: Failed to auto-detect serial port!" << ConsoleStyle() << std::endl;
            return -2;
        }
    }

    //open port with port name given on command line or filled by auto-detection
    if (!openSerialPort(portHandle, serialPortName, &oldOptions)) {
        return -2;
    }
    
    if (beVerbose) {
    	std::cout << "Sending command to Arduino..." << std::endl;
    }
    //terminate command with a line break
    commandData.push_back(COMMAND_TERMINATOR);
    //write command to port
	if (!writeToSerialPort(portHandle, commandData.data(), commandData.size() * sizeof(uint8_t))) {
		closeSerialPort(portHandle, &oldOptions);
		return -3;
	}
	
	if (beVerbose) {
    	std::cout << "Waiting for response from Arduino..." << std::endl;
    }

    //close port	
	closeSerialPort(portHandle, &oldOptions);

	return 0;
}

