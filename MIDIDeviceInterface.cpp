#include "MIDIDeviceInterface.h"

#include "MIDIWorker.h"
#include "rtmidi/RtMidi.h"
#include <QDebug>


MIDIDeviceInterface::MIDIDeviceInterface(QObject *parent)
	: QObject(parent)
	, m_midiIn(new RtMidiIn())
	, m_midiWorker(new MIDIWorker(m_midiIn))
{
	//do all possible connections to worker objects
	connect(&m_workerThread, &QThread::finished, m_midiWorker, &QObject::deleteLater);
	//connect returning signals
	connect(m_midiWorker, SIGNAL(midiMessage(double, const QByteArray &)), this, SLOT(messageReceived(double, const QByteArray &)));
	connect(m_midiWorker, SIGNAL(captureDeviceChanged(const QString &)), this, SLOT(inputDeviceChanged(const QString &)));
	connect(m_midiWorker, SIGNAL(captureStateChanged(bool)), this, SLOT(inputStateChanged(bool)));
	//move worker objects to thread and run thread
	m_midiWorker->moveToThread(&m_workerThread);
	m_workerThread.start();
}

MIDIDeviceInterface::~MIDIDeviceInterface()
{
	disconnect();
	m_workerThread.quit();
	m_workerThread.wait();
	delete m_midiIn;
}

void MIDIDeviceInterface::setCurrentCaptureDevice(const QString & inputName)
{
	QMetaObject::invokeMethod(m_midiWorker, "setCaptureDevice", Q_ARG(const QString &, inputName));
}

void MIDIDeviceInterface::setCaptureState(bool capture)
{
	QMetaObject::invokeMethod(m_midiWorker, "setCaptureState", Q_ARG(bool, capture));
}

bool MIDIDeviceInterface::isCapturing() const
{
	return m_midiWorker->isCapturing();
}

void MIDIDeviceInterface::inputDeviceChanged(const QString & deviceName)
{
	emit captureDeviceChanged(deviceName);
}

void MIDIDeviceInterface::inputStateChanged(bool capturing)
{
	emit captureStateChanged(capturing);
}

QStringList MIDIDeviceInterface::inputDeviceNames() const
{
	return m_midiWorker->inputDeviceNames();
}

QString MIDIDeviceInterface::defaultInputDeviceName() const
{
	return m_midiWorker->defaultInputDeviceName();
}

void MIDIDeviceInterface::messageReceived(double deltaTime, const QByteArray & message)
{
	if (message.size() >= 3)
	{
		//qDebug() << "MIDI message" << message;
		//check if it is a controller message
		const unsigned char type = message.at(0);
		if ((type & 0xF0) == 0xB0)
		{
			//split data
			const unsigned char controller = message.at(1);
			const QByteArray data = message.mid(2);
			emit midiControlMessage(deltaTime, controller, data);
		}
	}
}
