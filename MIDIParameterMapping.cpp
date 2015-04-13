#include "MIDIParameterMapping.h"

#include <QAbstractSlider>
#include <QAbstractButton>
#include <stdexcept>


MIDIParameterMapping::MIDIParameterMapping(QObject * parent)
	: QObject(parent)
	, m_mutex(QMutex::Recursive)
	, deviceName("deviceName", "")
	, learnMode("learnMode", false)
	, m_learnedGuiSide(false)
	, m_learnedMidiSide(false)
{
	connect(learnMode.GetSharedParameter().get(), SIGNAL(valueChanged(bool)), this, SLOT(setLearnMode(bool)));
}

void MIDIParameterMapping::toXML(QDomElement & parent) const
{
	QMutexLocker locker(&m_mutex);
	//try to find element in parent
	QDomNodeList children = parent.elementsByTagName("MIDIParameterMapping");
	for (int i = 0; i < children.size(); ++i)
	{
		QDomElement child = children.at(i).toElement();
		if (!child.isNull() && child.attribute("deviceName") == deviceName)
		{
			//remove child from document, we'll re-add it
			parent.removeChild(child);
			break;
		}
	}
	//(re-)add the new element
	QDomElement element = parent.ownerDocument().createElement("MIDIParameterMapping");
	deviceName.toXML(element);
	foreach(const MIDIParameterConnection & connection, m_connections)
	{
		connection.toXML(element);
	}
	parent.appendChild(element);
}

MIDIParameterMapping & MIDIParameterMapping::fromXML(const QDomElement & parent)
{
	QMutexLocker locker(&m_mutex);
	setLearnMode(false);
	//try to find element in document
	QDomNodeList mappings = parent.elementsByTagName("MIDIParameterMapping");
	if (mappings.isEmpty())
	{
		throw std::runtime_error("No MIDI mappings found!");
	}
	//try to find devicename in children
	for (int j = 0; j < mappings.size(); ++j)
	{
		QDomElement child = mappings.at(j).toElement();
		if (!child.isNull() && child.attribute("deviceName") == deviceName)
		{
			//found. read connections
			clearConnections();
			QDomNodeList connections = child.childNodes();
			for (int i = 0; i < connections.size(); ++i)
			{
				try
				{
					MIDIParameterConnection connection;
					QDomElement connectionElement = connections.at(i).toElement();
					connection.fromXML(connectionElement);
					//wow. that worked. try to find object in list
					foreach(const ControlEntry & control, m_controls)
					{
						if (control.parameter->name() == connection.m_parameterName && control.parentName == connection.m_parameterParentName)
						{
							//objects' name matches. store pointer in connection
							connection.m_parameter = control.parameter;
							connection.m_parameterParentName = control.parentName;
							m_connections.append(connection);
							break;
						}
					}
				}
				catch (std::runtime_error e)
				{
					//simply ignore unknown/bad nodes...
				}
			}
			return *this;
		}
	}
	throw std::runtime_error("No MIDI mappings found for current device!");
}

void MIDIParameterMapping::registerMIDIParameter(NodeRanged::SPtr parameter, const QString & parameterParentName)
{
	QMutexLocker locker(&m_mutex);
	//check for duplicate adds
	for (int i = 0; i < m_controls.size(); ++i)
	{
		if (m_controls.at(i).parameter == parameter)
		{
			//update parent name
			m_controls[i].parentName = parameterParentName;
			return;
		}
	}
	//add entry to list of controls
	ControlEntry control;
	control.parameter = parameter;
	control.parentName = parameterParentName;
	m_controls.append(control);
	connect(parameter.get(), SIGNAL(changed(NodeBase *)), this, SLOT(parameterChanged(NodeBase *)));
}

void MIDIParameterMapping::parameterChanged(NodeBase * parameter)
{
	QMutexLocker locker(&m_mutex);
	if (learnMode)
	{
		//try to upcast input parameter to a ranged one
		NodeRanged * rangedParameter = dynamic_cast<NodeRanged*>(parameter);
		if (rangedParameter)
		{
			//find sender in object list
			foreach(const ControlEntry & control, m_controls)
			{
				if (control.parameter.get() == rangedParameter)
				{
					m_learnConnection.m_parameter = control.parameter;
					m_learnConnection.m_parameterParentName = control.parentName;
					if (!m_learnedGuiSide)
					{
						m_learnedGuiSide = true;
						emit learnedConnectionStateChanged(m_learnedGuiSide && m_learnedMidiSide);
					}
					break;
				}
			}
		}
	}
}

void MIDIParameterMapping::midiControlMessage(double /*deltaTime*/, unsigned char controller, const QByteArray & data)
{
	QMutexLocker locker(&m_mutex);
	if (learnMode)
	{
		m_learnConnection.m_controller = controller;
		if (!m_learnedMidiSide)
		{
			m_learnedMidiSide = true;
			emit learnedConnectionStateChanged(m_learnedGuiSide && m_learnedMidiSide);
		}
		//if we have already learned the both GUI side and MIDI side, change the GUI value
		if (m_learnedGuiSide && m_learnedMidiSide)
		{
			//this does not take into account LSB/MSB-values, but we'll leave it like this for now...
			const float value = (float)data.at(0) / 127.0f;
			//set new value in object
			m_learnConnection.m_parameter->setNormalizedValue(value);
		}
	}
	else
	{
		if (data.size() > 0)
		{
			//find connection in the list
			foreach (const MIDIParameterConnection & connection, m_connections)
			{
				if (connection.m_controller == controller)
				{
					//this does not take into account LSB/MSB-values, but we'll leave it like this for now...
					const float value = (float)data.at(0) / 127.0f;
					//set new value in object
					connection.m_parameter->setNormalizedValue(value);
				}
			}
		}
	}
}

void MIDIParameterMapping::addConnection(unsigned char controller, NodeRanged::SPtr parameter, const QString & parameterParentName)
{
	QMutexLocker locker(&m_mutex);
	//check if connection is already in the list
	MIDIParameterConnection newConnnection(controller, parameter, parameterParentName);
	bool found = false;
	foreach(const MIDIParameterConnection & connection, m_connections)
	{
		if (connection == newConnnection)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		//add connection to list
		m_connections.append(newConnnection);
	}
}

void MIDIParameterMapping::clearConnections()
{
	QMutexLocker locker(&m_mutex);
	m_connections.clear();
}

void MIDIParameterMapping::setLearnMode(bool learn)
{
	QMutexLocker locker(&m_mutex);
	m_learnConnection.m_parameter.reset();
	m_learnConnection.m_parameterName = "";
	m_learnConnection.m_parameterParentName = "";
	m_learnConnection.m_controller = 0;
	m_learnedGuiSide = false;
	m_learnedMidiSide = false;
	learnMode = learn;
	emit learnedConnectionStateChanged(false);
}

void MIDIParameterMapping::storeLearnedConnection()
{
	QMutexLocker locker(&m_mutex);
	if (learnMode && m_learnedGuiSide && m_learnedMidiSide)
	{
		//store connection
		addConnection(m_learnConnection.m_controller, m_learnConnection.m_parameter);
		//clear connection for next round
		m_learnConnection.m_parameter.reset();
		m_learnConnection.m_parameterName = "";
		m_learnConnection.m_parameterParentName = "";
		m_learnConnection.m_controller = 0;
		m_learnedGuiSide = false;
		m_learnedMidiSide = false;
		emit learnedConnectionStateChanged(false);
	}
}
