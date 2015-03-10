#pragma once

#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>
#include <QStringList>

class RtMidiIn;
class MIDIWorker;


class MIDIDeviceInterface : public QObject
{
	Q_OBJECT

public:
	MIDIDeviceInterface(QObject *parent = 0);
	~MIDIDeviceInterface();

	void setCurrentCaptureDevice(const QString & inputName);
	void setCaptureState(bool capture);
	bool isCapturing() const;

	QStringList inputDeviceNames() const;
	QString defaultInputDeviceName() const;

signals:
	void captureDeviceChanged(const QString & name);
	void captureStateChanged(bool capturing);
	void midiControlMessage(double deltaTime, unsigned char controller, const QByteArray & data);

protected slots:
	void inputDeviceChanged(const QString & deviceName);
	void inputStateChanged(bool capturing);
	void messageReceived(double deltaTime, const QByteArray & message);

private:
	RtMidiIn * m_midiIn;
	MIDIWorker * m_midiWorker;
	QThread m_workerThread;
};