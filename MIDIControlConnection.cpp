#include "MIDIControlConnection.h"


MIDIControlConnection::MIDIControlConnection()
	: m_controller(0)
	, m_object(NULL)
	, m_slot("")
	, m_controlName("")
{
}

MIDIControlConnection::MIDIControlConnection(unsigned char controller, QObject * object, QString slot, QString controlName)
	: m_controller(controller)
	, m_object(object)
	, m_slot(slot)
	, m_controlName(controlName)
{
}

bool operator==(const MIDIControlConnection & a, const MIDIControlConnection & b)
{
	return (a.m_controller == b.m_controller && a.m_object == b.m_object && a.m_slot == b.m_slot && a.m_controlName == b.m_controlName);
}

bool operator!=(const MIDIControlConnection & a, const MIDIControlConnection & b)
{
	return !(a == b);
}