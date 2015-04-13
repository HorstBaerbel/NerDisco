#include "MIDIInterface.h"

#include <QObject>


std::mutex MIDIInterface::s_mutex;

MIDIInterface::SPtr & MIDIInterface::getInstance()
{
	static MIDIInterface::SPtr s_instance = nullptr;
	std::lock_guard<std::mutex> lock(s_mutex);
	if (!s_instance)
	{
		s_instance.reset(new MIDIInterface());
	}
	return s_instance;
}

MIDIDeviceInterface * MIDIInterface::getDeviceInterface()
{
	return m_interface;
}

MIDIParameterMapping * MIDIInterface::getParameterMapping()
{
	return m_mapping;
}

MIDIInterface::MIDIInterface()
	: m_interface(new MIDIDeviceInterface())
	, m_mapping(new MIDIParameterMapping())
{
	connectParameter(m_interface->captureDevice, m_mapping->deviceName);
	QObject::connect(m_interface, SIGNAL(midiControlMessage(double, unsigned char, const QByteArray &)), m_mapping, SLOT(midiControlMessage(double, unsigned char, const QByteArray &)));
}

MIDIInterface::~MIDIInterface()
{
	m_interface->disconnect();
	m_mapping->disconnect();
	delete m_interface;
	delete m_mapping;
}
