#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>

#include "MIDIControlConnection.h"


/// @brief This class can map MIDI control messages to I_MIDIControl objects.
/// Just derive from I_MIDIControl and add the object using registerMIDIControlObject().
class MIDIControlMapping : public QObject
{
	Q_OBJECT

public:
	MIDIControlMapping(QObject * parent = NULL);

	/// @brief Register a GUI object in the mapper for loading a mapping and doing the actual mapping.
	/// @param object GUI object to register that implements the I_MIDIControl interface.
	/// @note Call this directly after setting up the UI and before loading or using a mapping for all controls you want to use!
	void registerMIDIControlObject(I_MIDIControl * object);

	QDomElement toXML() const;
	MIDIControlMapping & fromXML(const QDomNode & node);

public slots:
	/// @brief The MIDI device name changed. Set a new one.
	/// @param deviceName New MIDI device name.
	void setMidiDevice(const QString & deviceName);

	/// @brief The value of a control in the gui has changed and the new value is sent.
	/// @param control The control the signal is coming from.
	/// @param value New value.
	void controlChanged(I_MIDIControl * control, float value);

	/// @brief Notify the class that a new MIDI message was received.
	/// @param deltaTime Delta time to last event.
	/// @param controller MIDI controller identifier.
	/// @param data Message data.
	void midiControlMessage(double deltaTime, unsigned char controller, const QByteArray & data);

	/// @brief Add a manual connection from a MIDI control message to a QObject slot.
	/// @param object Object via which the control can be changed.
	/// @param controller MIDI controller identifier.
	void addConnection(I_MIDIControl * object, unsigned char controller);

	/// @brief Remove all current connections.
	void clearConnections();

	/// @brief Toggle learn mode for MIDI->GUI connections.
	/// @param learn Pass true to enable learn mode.
	/// @note In this mode a midi control event received will be connected to a GUI event received.
	/// If you want to store the connection made, call addCurrentConnection().
	void setLearnMode(bool learn);

	/// @brief Check if the mapper is in learning mode.
	/// @return True if in learning mode.
	bool isLearnMode() const;

	/// @brief If in learn mode, adds the currently established connection to the list of connections.
	void storeLearnedConnection();

signals:
	/// @brief Emitted in learn mode when the state of the current connection changes.
	/// @param valid True means the current connection is valid an can be stored using storeCurrentConnection().
	void learnedConnectionStateChanged(bool valid);

private:
	mutable QMutex m_mutex;
	QVector<I_MIDIControl*> m_controls;
	QString m_deviceName;
	QVector<MIDIControlConnection> m_connections;
	bool m_learnMode;
	MIDIControlConnection m_learnConnection;
	bool m_learnedGuiSide;
	bool m_learnedMidiSide;
};
