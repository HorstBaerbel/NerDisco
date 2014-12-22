#include "SerialInterface.h"

#include <iostream>
#include <cstring>
#include <unistd.h>

#include "consolestyle.h"


SerialInterface::SerialInterface(const std::string & portName, bool beVerbose)
	: m_thread(0), m_mutex(PTHREAD_MUTEX_INITIALIZER), m_active(false)
{
    //try opening serial port
    if (beVerbose) {
        std::cout << "Opening serial port " << portName << " ..." << std::endl;
    }
    m_portHandle = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_portHandle <= 0) {
		std::cout << ConsoleStyle(ConsoleStyle::RED) << "Error: Failed to open serial port " << portName << "!" << ConsoleStyle() << std::endl;
        return;
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

	std::cout << ConsoleStyle(ConsoleStyle::GREEN) << "Missile control available." << ConsoleStyle() << std::endl;
}

void * SerialInterface::controlLoop(void * obj)
{
    LauncherCommand lastCommand = NONE;
	MissileControl * control = reinterpret_cast<MissileControl *>(obj);
	//start thread loop
	while (control != nullptr && control->active) {
		//block/unblock mutex while reading/modifying command
		pthread_mutex_lock(&control->mutex);
		//check if a command was issued
		if (control->currentCommand != NONE) {
			//check if a STOP command is needed now
			if (control->currentRemainingTime != INT_MIN && control->currentRemainingTime <= 0) {
				control->currentCommand = STOP;
				control->currentRemainingTime = INT_MIN;
			}
			if (control->currentRemainingTime != INT_MIN && control->currentRemainingTime > 0) {
				control->currentRemainingTime -= controlInterval;
			}
			if (control->currentCommand != lastCommand) {
			    //if the launcher is not armed, ignore a FIRE command
			    if (control->currentCommand != FIRE || (control->currentCommand == FIRE && control->armed)) {
			        //copy command sequences to command buffer
			        uint8_t commandBuffer[64];
			        memset(commandBuffer, 0, sizeof(commandBuffer));		
			        memcpy(commandBuffer, sequences[control->currentCommand], 8);
			        //send command to device
			        int errnum = 0;
			        if (control->launcherInfo.model == LAUNCHER_M_S) {
				        //needed for M&S launchers
				        if ((errnum = libusb_control_transfer(control->usbLauncher, LIBUSB_DT_HID, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_RECIPIENT_ENDPOINT, 0x01, SEQUENCE_INITA, sizeof(SEQUENCE_INITA), usbControlTimeout) <= 0) ||
					        (errnum = libusb_control_transfer(control->usbLauncher, LIBUSB_DT_HID, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_RECIPIENT_ENDPOINT, 0x01, SEQUENCE_INITB, sizeof(SEQUENCE_INITB), usbControlTimeout) <= 0) ||
					        (errnum = libusb_control_transfer(control->usbLauncher, LIBUSB_DT_HID, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_RECIPIENT_ENDPOINT, 0x01, commandBuffer, 64, usbControlTimeout) <= 0)) {
				            //                                                   0x21,                      0x09,               0x02, 0x01
						        std::cout << ConsoleStyle(ConsoleStyle::RED) << "Failed to send command to device. Error: " << libusb_error_name(errnum) << "." << ConsoleStyle() << std::endl;
						        control->currentCommand = NONE;
						        control->currentRemainingTime = INT_MIN;
				        }
			        }
			        else if (control->launcherInfo.model == LAUNCHER_CHEEKY) {
				        //sufficient for Dream Cheeky launchers
				        if (errnum = libusb_control_transfer(control->usbLauncher, LIBUSB_DT_HID, LIBUSB_REQUEST_SET_CONFIGURATION, LIBUSB_RECIPIENT_ENDPOINT, 0x00, commandBuffer, 8, usbControlTimeout) <= 0) {
					        std::cout << ConsoleStyle(ConsoleStyle::RED) << "Failed to send command to device. Error: " << libusb_error_name(errnum) << "." << ConsoleStyle() << std::endl;
					        control->currentCommand = NONE;
					        control->currentRemainingTime = INT_MIN;
				        }
			        }
			    }
			}
			//if the command was to fire or stop, switch command to NONE
			if (control->currentCommand == STOP || control->currentCommand == FIRE) {
				control->currentCommand = NONE;
				control->currentRemainingTime = INT_MIN;
			}
			lastCommand = control->currentCommand;
		}
		//unlock mutex again
		pthread_mutex_unlock(&control->mutex);
		//sleep controlInterval between launcher commands
		usleep(controlInterval * 1000);
	}
}

bool SerialInterface::executeCommand(LauncherCommand command, int durationMs)
{
	if (isAvailable()) {
		//if the command is fire or stop, the duration is set to INT_MIN anyway
		if (command == NONE || command == FIRE || command == STOP) {
			durationMs = INT_MIN;
		}
		//else if the duration if smaller than 0, set it to INT_MIN too
		else if (durationMs < 0) {
		    durationMs = INT_MIN;
		}
		pthread_mutex_lock(&mutex);
		currentCommand = command;
		currentRemainingTime = durationMs;
		pthread_mutex_unlock(&mutex);
		return true;
	}
	return false;
}

bool SerialInterface::isOpen() const
{
	return (usbContext != nullptr && usbLauncher != nullptr && active);
}

SerialInterface::~SerialInterface()
{
	std::cout << "Shutting down serial interface." << std::endl;
	if (thread != 0) {
		active = false;
		pthread_join(thread, 0);
		thread = 0;
	}
    if (usbContext) {
        if (usbLauncher != nullptr) {
            libusb_release_interface(usbLauncher, 0);
            libusb_release_interface(usbLauncher, 1);
            libusb_close(usbLauncher);
            usbLauncher = nullptr;
        }
        libusb_exit(usbContext);
        usbContext = nullptr;
    }
}


