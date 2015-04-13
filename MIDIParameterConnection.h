#pragma once

#include <QString>
#include <QObject>
#include <QDomElement>
#include "NodeRanged.h"


class MIDIParameterConnection
{
public:
	unsigned char m_controller;
	QString m_parameterName;
	QString m_parameterParentName;
	NodeRanged::SPtr m_parameter;

	MIDIParameterConnection();
	MIDIParameterConnection(unsigned char controller, NodeRanged::SPtr parameter, const QString & parameterParentName);

	void toXML(QDomElement & parent) const;
	MIDIParameterConnection & fromXML(const QDomElement & element);

	friend bool operator==(const MIDIParameterConnection & a, const MIDIParameterConnection & b);
	friend bool operator!=(const MIDIParameterConnection & a, const MIDIParameterConnection & b);
};
