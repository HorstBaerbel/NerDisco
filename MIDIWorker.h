#pragma once

#include <QObject>
#include <QMutex>
#include <QStringList>

class RtMidiIn;


class MIDIWorker : public QObject
{
	Q_OBJECT

public:
	MIDIWorker(RtMidiIn * midiIn, QObject *parent = 0);
	~MIDIWorker();

	static void midiCallback(double deltatime, std::vector<unsigned char> * message, void * userData);
	void midiCallback(double deltatime, std::vector<unsigned char> * message);
	bool isCapturing() const;

	QStringList inputDeviceNames() const;
	QString defaultInputDeviceName() const;

public slots:
	void setCaptureDevice(const QString & deviceName);
	void setCaptureState(bool capture);

signals:
	void midiMessage(double deltaTime, const QByteArray & message);
	void captureDeviceChanged(const QString & deviceName);
	void captureStateChanged(bool capturing);

private:
	mutable QMutex m_mutex;
	RtMidiIn * m_midiIn;
	QString m_deviceName;
	unsigned int m_portNumber;
	bool m_capturing;
};