#pragma once

#include <QString>
#include <QObject>


class MIDIControlConnection
{
public:
	unsigned char m_controller;
	QObject * m_object;
	QString m_slot;
	QString m_controlName;

	MIDIControlConnection();
	MIDIControlConnection(unsigned char controller, QObject * object, QString slot, QString controlName);

	friend bool operator==(const MIDIControlConnection & a, const MIDIControlConnection & b);
	friend bool operator!=(const MIDIControlConnection & a, const MIDIControlConnection & b);
};
