#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>


/// @brief This class can map MIDI control messages to QObject slots.
/// When adding connections manually through addConnection() the slot can have an arbitrary name, 
/// but must have the sginature "void ...(const QString & controlName, float value);"
/// If the connection is learned via guiControlChanged(), make sure your QObject has a slot 
/// with the EXACT signature "void setValue(const QString & controlName, float value);".
/// All values range from from [0.0f,1.0f]. Buttons send 0.0f for disabled and 1.0f for enabled.
class MIDIControlMapper: public QObject
{
	Q_OBJECT

public:
	MIDIControlMapper(QObject * parent = NULL);

public slots:
	/// @brief The value of a control in the gui has changed and the new value is sent.
	/// @param controlName Name of control that has changed.
	/// @param value New value in the range [0,1].
	/// @note Here the sender() object will be determined automagically. To make sure connections are working,
    ///       the calling object MUST have a slot "void setValue(const QString & controlName, float value);"!
	void guiControlChanged(const QString & controlName, float value);

	/// @brief Notify the class the a new MIDI message was received.
	/// @param deltaTime Delta time to last event.
	/// @param controller MIDI controller identifier.
	/// @param data Message data.
	void midiControlMessage(double deltaTime, unsigned char controller, const QByteArray & data);

	/// @brief Add a manual connection from a MIDI control message to a QObject slot.
	/// @param object Object via which the control can be changed.
	/// @param slot Slot to change values of control. Must have the signature "void slot(const QString & controlName, float value);"!
	/// @param controlName Name of control that has changed.
	/// @param controller MIDI controller identifier.
	void addConnection(QObject * object, const QString & slot, const QString & controlName, unsigned char controller);

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
	struct Connection
	{
		unsigned char controller;
		QObject * object;
		QString slot;
		QString controlName;
	};
	QVector<Connection> m_connections;

	mutable QMutex m_mutex;
	bool m_learnMode;
	Connection m_learnConnection;
	bool m_learnedGuiSide;
	bool m_learnedMidiSide;
};
