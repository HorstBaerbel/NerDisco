#include "MIDIControlMapper.h"


MIDIControlMapper::MIDIControlMapper(QObject * parent)
	: QObject(parent)
	, m_mutex(QMutex::Recursive)
	, m_learnMode(false)
	, m_learnedGuiSide(false)
	, m_learnedMidiSide(false)
{
	m_learnConnection.object = NULL;
	m_learnConnection.slot = "setValue";
	m_learnConnection.controlName = "";
	m_learnConnection.controller = 0;
}

void MIDIControlMapper::guiControlChanged(const QString & controlName, float /*value*/)
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode)
	{
		if (sender())
		{
			m_learnConnection.object = sender();
			m_learnConnection.slot = "setValue";
			m_learnConnection.controlName = controlName;
			if (!m_learnedGuiSide)
			{
				m_learnedGuiSide = true;
				emit learnedConnectionStateChanged(m_learnedGuiSide && m_learnedMidiSide);
			}
		}
	}
}

void MIDIControlMapper::midiControlMessage(double /*deltaTime*/, unsigned char controller, const QByteArray & data)
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode)
	{
		m_learnConnection.controller = controller;
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
			QMetaObject::invokeMethod(m_learnConnection.object, m_learnConnection.slot.toLatin1(), Q_ARG(const QString &, m_learnConnection.controlName), Q_ARG(float, value));
		}
	}
	else
	{
		if (data.size() > 0)
		{
			//find connection in the list
			foreach(const Connection & connection, m_connections)
			{
				if (connection.controller == controller)
				{
					//this does not take into account LSB/MSB-values, but we'll leave it like this for now...
					const float value = (float)data.at(0) / 127.0f;
					QMetaObject::invokeMethod(connection.object, connection.slot.toLatin1(), Q_ARG(const QString &, connection.controlName), Q_ARG(float, value));
				}
			}
		}
	}
}

void MIDIControlMapper::addConnection(QObject * object, const QString & slot, const QString & controlName, unsigned char controller)
{
	QMutexLocker locker(&m_mutex);
	//check if connection is already in the list
	bool found = false;
	foreach(const Connection & connection, m_connections)
	{
		if (connection.object == object && connection.slot == slot && connection.controlName == controlName && connection.controller == controller)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		//add connection to list
		Connection newConnnection;
		newConnnection.object = object;
		newConnnection.slot = slot;
		newConnnection.controlName = controlName;
		newConnnection.controller = controller;
		m_connections.append(newConnnection);
	}
}

void MIDIControlMapper::clearConnections()
{
	QMutexLocker locker(&m_mutex);
	m_connections.clear();
}

void MIDIControlMapper::setLearnMode(bool learn)
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode != learn)
	{
		m_learnConnection.object = NULL;
		m_learnConnection.slot = "setValue";
		m_learnConnection.controlName = "";
		m_learnConnection.controller = 0;
		m_learnedGuiSide = false;
		m_learnedMidiSide = false;
		m_learnMode = learn;
		emit learnedConnectionStateChanged(false);
	}
}

bool MIDIControlMapper::isLearnMode() const
{
	QMutexLocker locker(&m_mutex);
	return m_learnMode;
}

void MIDIControlMapper::storeLearnedConnection()
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode && m_learnedGuiSide && m_learnedMidiSide)
	{
		//store connection
		addConnection(m_learnConnection.object, m_learnConnection.slot, m_learnConnection.controlName, m_learnConnection.controller);
		//clear connection for next round
		m_learnConnection.object = NULL;
		m_learnConnection.slot = "setValue";
		m_learnConnection.controlName = "";
		m_learnConnection.controller = 0;
		m_learnedGuiSide = false;
		m_learnedMidiSide = false;
		emit learnedConnectionStateChanged(false);
	}
}
