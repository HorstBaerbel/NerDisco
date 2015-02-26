#include "MIDIControlMapper.h"


MIDIControlMapper::MIDIControlMapper(QObject * parent)
	: QObject(parent)
	, m_mutex(QMutex::Recursive)
	, m_learnMode(false)
	, m_learnedGuiSide(false)
	, m_learnedMidiSide(false)
	, m_learnConnection(0, NULL, "aetValue", "")
{
}

void MIDIControlMapper::guiControlChanged(const QString & controlName, float /*value*/)
{
	QMutexLocker locker(&m_mutex);
	if (m_learnMode)
	{
		if (sender())
		{
			m_learnConnection.m_object = sender();
			m_learnConnection.m_slot = "setValue";
			m_learnConnection.m_controlName = controlName;
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
			QMetaObject::invokeMethod(m_learnConnection.m_object, m_learnConnection.m_slot.toLatin1(), Q_ARG(const QString &, m_learnConnection.m_controlName), Q_ARG(float, value));
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
					QMetaObject::invokeMethod(connection.m_object, connection.m_slot.toLatin1(), Q_ARG(const QString &, connection.m_controlName), Q_ARG(float, value));
				}
			}
		}
	}
}

void MIDIControlMapper::addConnection(QObject * object, const QString & slot, const QString & controlName, unsigned char controller)
{
	QMutexLocker locker(&m_mutex);
	//check if connection is already in the list
	MIDIControlConnection newConnnection(controller, object, slot, controlName);
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
		m_learnConnection.m_object = NULL;
		m_learnConnection.m_slot = "setValue";
		m_learnConnection.m_controlName = "";
		m_learnConnection.m_controller = 0;
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
		addConnection(m_learnConnection.m_object, m_learnConnection.m_slot, m_learnConnection.m_controlName, m_learnConnection.m_controller);
		//clear connection for next round
		m_learnConnection.m_object = NULL;
		m_learnConnection.m_slot = "setValue";
		m_learnConnection.m_controlName = "";
		m_learnConnection.m_controller = 0;
		m_learnedGuiSide = false;
		m_learnedMidiSide = false;
		emit learnedConnectionStateChanged(false);
	}
}
