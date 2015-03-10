#pragma once

#include "MIDIDeviceInterface.h"
#include "MIDIControlMapping.h"

#include <memory>
#include <mutex>


/// @brief Singleton class holding a MIDI interface for one device, consisting of a device interface and a mapper.
/// This is based on this (Version 3): http://silviuardelean.ro/2012/06/05/few-singleton-approaches/
/// and should be reasonably thread safe.
class MIDIInterface
{
public:
	/// brief Shared pointer of MIDIInterface object.
	typedef std::shared_ptr<MIDIInterface> SPtr;

	/// @brief Retrieve or create and instance of the MIDI interface.
	/// @return Shared pointer to MIDI interface.
	/// @note This also creates a MIDIDeviceInterface and MIDIControlMapping and connects the two.
	static SPtr & getInstance();

	/// @brief Retrieve MIDI device interface.
	/// @return Pointer to MIDI device interface object.
	MIDIDeviceInterface * getDeviceInterface();

	/// @brief Retrieve MIDI control mapping.
	/// @return Pointer to MIDI control mapping object.
	MIDIControlMapping * getControlMapping();

	/// @Destructor. We delete the QObjects here.
	~MIDIInterface();

private:
	MIDIInterface();
	MIDIInterface(MIDIInterface & mi);
	MIDIInterface & operator=(const MIDIInterface & mi);

	static std::mutex s_mutex;
	MIDIDeviceInterface * m_interface;
	MIDIControlMapping * m_mapping;
};
