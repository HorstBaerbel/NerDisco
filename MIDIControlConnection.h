#pragma once

#include <QString>
#include <QObject>
#include <QDomNode>
#include "I_MIDIControl.h"


class MIDIControlConnection
{
public:
	unsigned char m_controller;
	QString m_objectPathName;
	I_MIDIControl * m_object;

	MIDIControlConnection();
	MIDIControlConnection(unsigned char controller, I_MIDIControl * object);

	QDomElement toXML() const;
	MIDIControlConnection & fromXML(const QDomNode & node);

	friend bool operator==(const MIDIControlConnection & a, const MIDIControlConnection & b);
	friend bool operator!=(const MIDIControlConnection & a, const MIDIControlConnection & b);
};
