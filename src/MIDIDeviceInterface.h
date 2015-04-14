#pragma once

#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>
#include <QStringList>
#include <QDomDocument>
#include "Parameters.h"

class RtMidiIn;
class MIDIWorker;


class MIDIDeviceInterface : public QObject
{
	Q_OBJECT

public:
	MIDIDeviceInterface(QObject *parent = 0);
	~MIDIDeviceInterface();

	/// @brief Save the current settings to an XML document.
	/// @param parent The paren element to write the settings to.
	void toXML(QDomElement & parent) const;
	/// @brief Read current settings from XML document.
	/// @param parent The parent element to load the settings from.
	MIDIDeviceInterface & fromXML(const QDomElement & parent);

	ParameterQString captureDevice;
	ParameterBool capturing;

	QStringList inputDeviceNames() const;
	QString defaultInputDeviceName() const;

signals:
	void midiControlMessage(double deltaTime, unsigned char controller, const QByteArray & data);

protected slots:
	void setCaptureDevice(const QString & inputName);
	void setCaptureState(bool capture);

	void messageReceived(double deltaTime, const QByteArray & message);

private:
	RtMidiIn * m_midiIn;
	MIDIWorker * m_midiWorker;
	QThread m_workerThread;
};