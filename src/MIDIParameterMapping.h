#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>

#include "MIDIParameterConnection.h"
#include "Parameters.h"


/// @brief This class can map MIDI control messages to I_MIDIControl objects.
/// Just derive from I_MIDIControl and add the object using registerMIDIParameter().
class MIDIParameterMapping : public QObject
{
	Q_OBJECT

public:
	MIDIParameterMapping(QObject * parent = NULL);

	/// @brief Register a parameter in the mapper for loading a mapping and doing the actual mapping.
	/// @param parameter Parameter to register.
	/// @param parameterParentName Name of parent of parameter. Use if you have parameters of the same name with different parents.
	/// @note Call this before loading a mapping or using a mapping for all controls you want to use!
	void registerMIDIParameter(NodeRanged::SPtr parameter, const QString & parameterParentName = "");

	/// @brief Save the current device mapping to an XML document.
	/// @param parent The parent to append the mapping to.
	void toXML(QDomElement & parent) const;
	/// @brief Read device mapping for the currently set device from XML document.
	/// @param parent The parent element to load the mapping from.
	/// @note this will throw std::runtime_error if no mapping with the current device name is found in the document.
	MIDIParameterMapping & fromXML(const QDomElement & parent);

	/// @brief The name of the MIDI device this mapping is for. This used for serializing to/from XML.
	ParameterQString deviceName;
	/// @brief If set to true the mapping will monitor registered parameters and the MIDI controller
	/// and associate both when you storeLernedConnection(). If a connection is valid lernedConnectionStateChanged() is emitted.
	ParameterBool learnMode;

public slots:
	/// @brief The value of a parameter has changed.
	/// @param parameter The parameter the signal is coming from.
	void parameterChanged(NodeBase * parameter);

	/// @brief Notify the class that a new MIDI message was received.
	/// @param deltaTime Delta time to last event.
	/// @param controller MIDI controller identifier.
	/// @param data Message data.
	void midiControlMessage(double deltaTime, unsigned char controller, const QByteArray & data);

	/// @brief Add a manual connection from a MIDI control message to a parameter.
	/// @param controller MIDI controller identifier.
	/// @param parameter Parameter the midi control should change.
	/// @param parameterParentName Name of parent of parameter. Use if you have parameters of the same name with different parents.
	void addConnection(unsigned char controller, NodeRanged::SPtr parameter, const QString & parameterParentName = "");

	/// @brief Remove all current connections.
	void clearConnections();

	/// @brief If in learn mode, adds the currently established connection to the list of connections.
	void storeLearnedConnection();

signals:
	/// @brief Emitted in learn mode when the state of the current connection changes.
	/// @param valid True means the current connection is valid an can be stored using storeCurrentConnection().
	void learnedConnectionStateChanged(bool valid);

private slots:
	void setLearnMode(bool learn);

private:
	mutable QMutex m_mutex;

	struct ControlEntry
	{
		NodeRanged::SPtr parameter;
		QString parentName;
	};
	QVector<ControlEntry> m_controls;

	QVector<MIDIParameterConnection> m_connections;
	MIDIParameterConnection m_learnConnection;
	bool m_learnedGuiSide;
	bool m_learnedMidiSide;
};
