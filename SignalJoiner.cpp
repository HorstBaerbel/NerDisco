#include "SignalJoiner.h"


SignalJoiner::~SignalJoiner()
{
	disconnect();
	m_waitList.clear();
	m_objectList.clear();
}

void SignalJoiner::addObjectToJoin(QObject * object)
{
	QMutexLocker locker(&m_listMutex);
	if (!m_waitList.isEmpty())
	{
		throw std::exception("Can not add objects while joiner is running!");
	}
	if (!m_objectList.contains(object))
	{
		m_objectList.append(object);
	}
}

void SignalJoiner::removeObjectToJoin(QObject * object)
{
	QMutexLocker locker(&m_listMutex);
	if (!m_waitList.isEmpty())
	{
		throw std::exception("Can not remove objects while joiner is running!");
	}
	if (m_objectList.indexOf(object) != -1)
	{
		object->disconnect(this, SLOT(notify()));
		m_objectList.remove(m_objectList.indexOf(object));
	}
}

void SignalJoiner::start()
{
	QMutexLocker locker(&m_listMutex);
	if (m_objectList.isEmpty())
	{
		throw std::exception("Object list empty. Add objects to join first!");
	}
	m_waitList = m_objectList;
}

void SignalJoiner::stop()
{
	m_waitList.clear();
}

bool SignalJoiner::isJoining() const
{
	QMutexLocker locker(&m_listMutex);
	return (!m_waitList.isEmpty());
}

void SignalJoiner::notify()
{
	QMutexLocker locker(&m_listMutex);
	if (!m_waitList.isEmpty())
	{
		QObject * callingObject = sender();
		if (callingObject && m_waitList.indexOf(callingObject) != -1)
		{
			m_waitList.remove(m_waitList.indexOf(callingObject));
			if (m_waitList.isEmpty())
			{
				emit joined();
			}
		}
	}
}
