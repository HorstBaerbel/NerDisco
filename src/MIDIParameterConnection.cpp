#include "MIDIParameterConnection.h"

#include <stdexcept>


MIDIParameterConnection::MIDIParameterConnection()
	: m_controller(0)
{
}

MIDIParameterConnection::MIDIParameterConnection(unsigned char controller, NodeRanged::SPtr parameter, const QString & parameterParentName)
	: m_controller(controller)
	, m_parameter(parameter)
	, m_parameterParentName(parameterParentName)
{
}

void MIDIParameterConnection::toXML(QDomElement & parent) const
{
	QDomElement element = parent.ownerDocument().createElement("MIDIParameterConnection");
	//build name from parent + control
	element.setAttribute("parameterName", m_parameter->name());
	element.setAttribute("parameterParentName", m_parameterParentName);
	element.setAttribute("controller", m_controller);
	parent.appendChild(element);
}

MIDIParameterConnection & MIDIParameterConnection::fromXML(const QDomElement & element)
{
	if (element.tagName() != "MIDIParameterConnection")
	{
		throw std::runtime_error("Node is not a MIDIParameterConnection");
	}
	m_parameter.reset();
	m_parameterName = element.attribute("parameterName");
	m_parameterParentName = element.attribute("parameterParentName");
	m_controller = element.attribute("controller", 0).toUInt();
	return *this;
}

bool operator==(const MIDIParameterConnection & a, const MIDIParameterConnection & b)
{
	return (a.m_controller == b.m_controller && a.m_parameter == b.m_parameter && a.m_parameterName == b.m_parameterName && a.m_parameterParentName == b.m_parameterParentName);
}

bool operator!=(const MIDIParameterConnection & a, const MIDIParameterConnection & b)
{
	return !(a == b);
}
