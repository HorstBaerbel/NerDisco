#include "MIDIInterface.h"

#include "MIDIWorker.h"
#include "rtmidi/RtMidi.h"
#include <QDebug>


MIDIInterface::MIDIInterface(QObject *parent)
	: QObject(parent)
	, m_midiIn(new RtMidiIn())
	, m_midiWorker(new MIDIWorker(m_midiIn))
{
	//register metatype so all signal/slot connections work
	//qRegisterMetaType< QVector<float> >("QVector<float>");
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

MIDIInterface::~MIDIInterface()
{
	m_workerThread.quit();
	m_workerThread.wait();
	delete m_midiIn;
}

void MIDIInterface::setCurrentCaptureDevice(const QString & inputName)
{
	QMetaObject::invokeMethod(m_midiWorker, "setCaptureDevice", Q_ARG(const QString &, inputName));
}

void MIDIInterface::setCaptureState(bool capture)
{
	QMetaObject::invokeMethod(m_midiWorker, "setCaptureState", Q_ARG(bool, capture));
}

bool MIDIInterface::isCapturing() const
{
	return m_midiWorker->isCapturing();
}

void MIDIInterface::inputDeviceChanged(const QString & deviceName)
{
	emit captureDeviceChanged(deviceName);
}

void MIDIInterface::inputStateChanged(bool capturing)
{
	emit captureStateChanged(capturing);
}

QStringList MIDIInterface::inputDeviceNames() const
{
	return m_midiWorker->inputDeviceNames();
}

QString MIDIInterface::defaultInputDeviceName() const
{
	return m_midiWorker->defaultInputDeviceName();
}

void MIDIInterface::connectToControllerMessage(unsigned char controller, QObject * receiver, const char * slot, const QString & userData)
{
	//sanity checks
	if (receiver != NULL && slot != NULL)
	{
		QMutexLocker locker(&m_mutex);
		//check if entry is already in list
		Receiver newEntry;
		newEntry.controller = controller;
		newEntry.object = receiver;
		newEntry.slot = slot;
		newEntry.userData = userData;
		bool found = false;
		for (int i = 0; i < m_registeredReceivers.size(); ++i)
		{
			if (m_registeredReceivers.at(i).controller == controller && m_registeredReceivers.at(i).object == receiver && m_registeredReceivers.at(i).slot == slot)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			//add receiver to list
			m_registeredReceivers.append(newEntry);
		}
	}
}

void MIDIInterface::disconnectFromControllerMessage(QObject * receiver, const char * slot)
{
	QMutexLocker locker(&m_mutex);
	for (int i = 0; i < m_registeredReceivers.size(); ++i)
	{
		if (m_registeredReceivers.at(i).object == receiver && (slot == NULL || m_registeredReceivers.at(i).slot == slot))
		{
			m_registeredReceivers.removeAt(i);
		}
	}
}

void MIDIInterface::messageReceived(double /*deltaTime*/, const QByteArray & message)
{
	if (message.size() >= 3)
	{
		qDebug() << "MIDI message" << message;
		//check if it is a controller message
		const unsigned char type = message.at(0);
		if ((type & 0xF0) == 0xB0)
		{
			QMutexLocker locker(&m_mutex);
			//split data
			const unsigned char controller = message.at(1);
			const QByteArray data = message.mid(2);
			//check if the controller is registered
			foreach(const Receiver & receiver, m_registeredReceivers)
			{
				//if controller is registered, invoke slot
				if (receiver.controller == controller)
				{
					QMetaObject::invokeMethod(receiver.object, receiver.slot, Q_ARG(const QByteArray &, data), Q_ARG(const QString &, receiver.userData));
				}
			}
		}
	}
}
