#include "MIDIControlConnection.h"

#include <stdexcept>


MIDIControlConnection::MIDIControlConnection()
	: m_controller(0)
	, m_object(NULL)
{
}

MIDIControlConnection::MIDIControlConnection(unsigned char controller, I_MIDIControl * object)
	: m_controller(controller)
	, m_object(object)
{
}

QDomElement MIDIControlConnection::toXML() const
{
	QDomElement element;
	element.setTagName("MIDIControlConnection");
	//build name from parent + control
	QString objectPathName = m_object->toQObject()->parent()->objectName() + "." + m_object->toQObject()->objectName();
	element.setAttribute("objectPathName", objectPathName);
	element.setAttribute("controller", m_controller);
	return element;
}

MIDIControlConnection & MIDIControlConnection::fromXML(const QDomNode & node)
{
	QDomElement element = node.toElement();
	if (element.isNull())
	{
		throw std::runtime_error("Node is not an QDomElement");
	}
	if (element.tagName() != "MIDIControlConnection")
	{
		throw std::runtime_error("Node is not a MIDIControlConnection");
	}
	m_object = NULL;
	m_objectPathName = element.attribute("objectPathName");
	m_controller = element.attribute("controller", 0).toUInt();
	return *this;
}

bool operator==(const MIDIControlConnection & a, const MIDIControlConnection & b)
{
	return (a.m_controller == b.m_controller && a.m_object == b.m_object && a.m_objectPathName == b.m_objectPathName);
}

bool operator!=(const MIDIControlConnection & a, const MIDIControlConnection & b)
{
	return !(a == b);
}
