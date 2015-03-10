#include "MIDIControlMapping.h"

#include <QAbstractSlider>
#include <QAbstractButton>
#include <stdexcept>


MIDIControlMapping::MIDIControlMapping(QObject * parent)
	: QObject(parent)
	, m_mutex(QMutex::Recursive)
	, m_learnMode(false)
	, m_learnedGuiSide(false)
	, m_learnedMidiSide(false)
	, m_learnConnection(0, NULL)
{
}

QDomElement MIDIControlMapping::toXML() const
{
	QMutexLocker locker(&m_mutex);
	QDomElement element;
	element.setTagName("MIDIControlMapping");
	element.setAttribute("deviceName", m_deviceName);
	foreach(const MIDIControlConnection & connection, m_connections)
	{
		element.appendChild(connection.toXML());
	}
	return element;
}

MIDIControlMapping & MIDIControlMapping::fromXML(const QDomNode & node)
{
	QMutexLocker locker(&m_mutex);
	setLearnMode(false);
	QDomElement element = node.toElement();
	if (element.isNull())
	{
		throw std::runtime_error("Node is not a QDomElement");
	}
	if (element.tagName() != "MIDIControlMapping")
	{
		throw std::runtime_error("Node is not a MIDIControlMapping");
	}
	m_deviceName = element.attribute("deviceName");
	clearConnections();
	QDomNodeList children = element.childNodes();
	for (int i = 0; i < children.size(); ++i)
	{
		try
		{
			MIDIControlConnection connection;
			connection.fromXML(children.at(i));
			//wow. that worked. try to find object in list
			foreach (I_MIDIControl * object, m_controls)
			{
				//build name from parent + control
				const QString objectPathName = object->toQObject()->parent()->objectName() + "." + object->toQObject()->objectName();
				if (objectPathName == connection.m_objectPathName)
				{
					//objects' name matches. store pointer in connection
					connection.m_object = object;
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

void MIDIControlMapping::registerMIDIControlObject(I_MIDIControl * object)
{
	QMutexLocker locker(&m_mutex);
	if (!m_controls.contains(object))
	{
		m_controls.append(object);
		connect(object->toQObject(), SIGNAL(guiValueChanged(I_MIDIControl *, float)), this, SLOT(controlChanged(I_MIDIControl *, float)));
	}
}

void MIDIControlMapping::setMidiDevice(const QString & deviceName)
{
	m_deviceName = deviceName;
}

void MIDIControlMapping::controlChanged(I_MIDIControl * control, float /*value*/)
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode)
	{
		if (control)
		{
			//find sender in object list
			foreach (I_MIDIControl * object, m_controls)
			{
				if (object == control)
				{
					m_learnConnection.m_object = object;
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

void MIDIControlMapping::midiControlMessage(double /*deltaTime*/, unsigned char controller, const QByteArray & data)
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode)
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
			m_learnConnection.m_object->setValueFromMIDI(value);
		}
	}
	else
	{
		if (data.size() > 0)
		{
			//find connection in the list
			foreach(const MIDIControlConnection & connection, m_connections)
			{
				if (connection.m_controller == controller)
				{
					//this does not take into account LSB/MSB-values, but we'll leave it like this for now...
					const float value = (float)data.at(0) / 127.0f;
					//set new value in object
					connection.m_object->setValueFromMIDI(value);
				}
			}
		}
	}
}

void MIDIControlMapping::addConnection(I_MIDIControl * object, unsigned char controller)
{
	QMutexLocker locker(&m_mutex);
	//check if connection is already in the list
	MIDIControlConnection newConnnection(controller, object);
	bool found = false;
	foreach(const MIDIControlConnection & connection, m_connections)
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

void MIDIControlMapping::clearConnections()
{
	QMutexLocker locker(&m_mutex);
	m_connections.clear();
}

void MIDIControlMapping::setLearnMode(bool learn)
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode != learn)
	{
		m_learnConnection.m_object = NULL;
		m_learnConnection.m_objectPathName = "";
		m_learnConnection.m_controller = 0;
		m_learnedGuiSide = false;
		m_learnedMidiSide = false;
		m_learnMode = learn;
		emit learnedConnectionStateChanged(false);
	}
}

bool MIDIControlMapping::isLearnMode() const
{
	QMutexLocker locker(&m_mutex);
	return m_learnMode;
}

void MIDIControlMapping::storeLearnedConnection()
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode && m_learnedGuiSide && m_learnedMidiSide)
	{
		//store connection
		addConnection(m_learnConnection.m_object, m_learnConnection.m_controller);
		//clear connection for next round
		m_learnConnection.m_object = NULL;
		m_learnConnection.m_objectPathName = "";
		m_learnConnection.m_controller = 0;
		m_learnedGuiSide = false;
		m_learnedMidiSide = false;
		emit learnedConnectionStateChanged(false);
	}
}
