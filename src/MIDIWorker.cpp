#include "MIDIWorker.h"

#include "rtmidi/RtMidi.h"


MIDIWorker::MIDIWorker(RtMidiIn * midiIn, QObject * parent)
	: QObject(parent)
	, m_midiIn(midiIn)
	, m_portNumber(0)
	, m_capturing(false)
{
}

MIDIWorker::~MIDIWorker()
{
}

void MIDIWorker::midiCallback(double deltatime, std::vector<unsigned char> * message, void * userData)
{
	if (userData)
	{
		MIDIWorker * worker = reinterpret_cast<MIDIWorker*>(userData);
		worker->midiCallback(deltatime, message);
	}
}

void MIDIWorker::midiCallback(double deltatime, std::vector<unsigned char> * message)
{
	QByteArray array = QByteArray::fromRawData(reinterpret_cast<const char *>(message->data()), static_cast<int>(message->size()));
	emit midiMessage(deltatime, array);
}

void MIDIWorker::setCaptureDevice(const QString & deviceName)
{
	QMutexLocker locker(&m_mutex);
	if (m_midiIn && m_deviceName != deviceName)
	{
		//check if we're currently capturing from the device and stop
		m_midiIn->closePort();
		m_midiIn->setCallback(NULL, NULL);
		emit captureStateChanged(false);
		//try to find device name in port list
		const QStringList list = inputDeviceNames();
		if (list.indexOf(deviceName) >= 0)
		{
			m_deviceName = deviceName;
			m_portNumber = list.indexOf(m_deviceName);
		}
		else
		{
			m_deviceName.clear();
			m_portNumber = 0;
		}
		emit captureDeviceChanged(m_deviceName);
	}
}

QString MIDIWorker::captureDevice() const
{
	QMutexLocker locker(&m_mutex);
	return m_deviceName;
}

void MIDIWorker::setCaptureState(bool capture)
{
	QMutexLocker locker(&m_mutex);
	if (m_midiIn && m_midiIn->isPortOpen() != capture)
	{
		if (capture)
		{
			//check for a valid port number
			if (!m_deviceName.isEmpty() && m_midiIn->getPortCount() > m_portNumber)
			{
				//try to open port and check if it worked
				m_midiIn->openPort(m_portNumber);
				if (m_midiIn->isPortOpen())
				{
					m_midiIn->setCallback(&MIDIWorker::midiCallback, this);
					emit captureStateChanged(true);
				}
			}
		}
		else
		{
			//try to close port and check if it worked
			m_midiIn->closePort();
			m_midiIn->setCallback(NULL, NULL);
			emit captureStateChanged(false);
		}
	}
}

bool MIDIWorker::isCapturing() const
{
	QMutexLocker locker(&m_mutex);
	return (m_midiIn && m_midiIn->isPortOpen());
}

QStringList MIDIWorker::inputDeviceNames() const
{
	QStringList names;
	unsigned int nPorts = m_midiIn->getPortCount();
	for (unsigned int i = 0; i < nPorts; i++)
	{
		try
		{
			QString portName = QString::fromStdString(m_midiIn->getPortName(i));
			names.append(portName);
		}
		catch (RtMidiError & /*error*/)
		{
		}
	}
	return names;
}

QString MIDIWorker::defaultInputDeviceName() const
{
	unsigned int nPorts = m_midiIn->getPortCount();
	if (nPorts > 0)
	{
		try
		{
			return QString::fromStdString(m_midiIn->getPortName(0));
		}
		catch (RtMidiError & /*error*/)
		{
		}
	}
	return QString();
}
