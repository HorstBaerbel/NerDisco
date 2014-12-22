#pragma once

#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>


class SerialInterface
{
public:
    /*!
    Open serial port.
    \param[in] portName Serial port device name.
    \param[in] beVerbose Pass true to output more information.
    */
	SerialInterface(const std::string & portName, bool beVerbose = false);

    /*!
    Check if the port is open and we can send data.
    \return Returns true if the port is open.
    */	
	bool isOpen() const;

	/*!
    Sends data to the port.
	\param[in] data The data to send.
	\return Returns true if sending was successful, false if not.
	*/
	bool sendData(std::vector<uint8_t> data);

	~SerialInterface();

private:
    /*!
    The worker function sending the actual data.
    */
    static void * sendLoop(void * obj);

    pthread_t m_thread; //!<serial sending thread.
	pthread_mutex_t m_mutex; //!<The mutex protecting the member variables.
    bool m_active; //!<flags to keep the thread running or stop it.
    int & m_portHandle; //!<Serial port handle.
    const std::string m_portName; //!<Serial port device name if open.
    termios * m_oldOptions; //!<TERMIOS options of port before we changed them.
};

