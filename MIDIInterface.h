#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QString>
#include <QVector>
#include <QStringList>

class RtMidiIn;
class MIDIWorker;


class MIDIInterface : public QObject
{
	Q_OBJECT

public:
	MIDIInterface(QObject *parent = 0);
	~MIDIInterface();

	void setCurrentCaptureDevice(const QString & inputName);
	void setCaptureState(bool capture);
	bool isCapturing() const;

	QStringList inputDeviceNames() const;
	QString defaultInputDeviceName() const;

	/// @brief Connect slot to be called when a MIDI message arrives on a specific controller.
	/// @param controller Controller number to watch.
	/// @param receiver Object that will receive the signal.
	/// @param slot Slot that will receive the notification. The signature must be void slot(const QByteArray & data, const QString & userData)
	/// @param userData Optional. User data that will be passed to the slot.
	void connectToControllerMessage(unsigned char controller, QObject * receiver, const char * slot, const QString & userData = QString());
	/// @brief Disconnect a receiving slot from all messages.
	/// @param receiver Object to disconnect.
	/// @param slot Slot to disconnect.
	void disconnectFromControllerMessage(QObject * receiver, const char * slot);

signals:
	void captureDeviceChanged(const QString & name);
	void captureStateChanged(bool capturing);

protected slots:
	void inputDeviceChanged(const QString & deviceName);
	void inputStateChanged(bool capturing);
	void messageReceived(double deltaTime, const QByteArray & message);

private:
	RtMidiIn * m_midiIn;
	MIDIWorker * m_midiWorker;
	QThread m_workerThread;
	QMutex m_mutex;

	struct Receiver
	{
		unsigned char controller;
		QObject * object;
		const char * slot;
		QString userData;
	};
	QVector<Receiver> m_registeredReceivers;
};