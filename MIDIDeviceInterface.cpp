#include "MIDIDeviceInterface.h"

#include "MIDIWorker.h"
#include "rtmidi/RtMidi.h"
#include <QDebug>


MIDIDeviceInterface::MIDIDeviceInterface(QObject *parent)
	: QObject(parent)
	, m_midiIn(new RtMidiIn())
	, m_midiWorker(new MIDIWorker(m_midiIn))
	, captureDevice("captureDevice", "")
	, capturing("capturing", false)
{
	//do all possible connections to worker objects
	connect(&m_workerThread, &QThread::finished, m_midiWorker, &QObject::deleteLater);
	//connect returning signals
	connect(m_midiWorker, SIGNAL(midiMessage(double, const QByteArray &)), this, SLOT(messageReceived(double, const QByteArray &)));
	connect(m_midiWorker, SIGNAL(captureDeviceChanged(const QString &)), captureDevice.GetSharedParameter().get(), SLOT(setValue(const QString &)));
	connect(m_midiWorker, SIGNAL(captureStateChanged(bool)), capturing.GetSharedParameter().get(), SLOT(setValue(bool)));
	connect(captureDevice.GetSharedParameter().get(), SIGNAL(valueChanged(const QString &)), this, SLOT(setCaptureDevice(const QString &)));
	connect(capturing.GetSharedParameter().get(), SIGNAL(valueChanged(bool)), this, SLOT(setCaptureState(bool)));
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

void MIDIDeviceInterface::toXML(QDomElement & parent) const
{
	//try to find element in document
	QDomNodeList children = parent.elementsByTagName("MIDIDeviceInterface");
	for (int i = 0; i < children.size(); ++i)
	{
		QDomElement child = children.at(i).toElement();
		if (!child.isNull() && child.attribute("deviceName") == m_midiWorker->captureDevice())
		{
			//remove child from document, we'll re-add it
			parent.removeChild(child);
			break;
		}
	}
	//(re-)add the new element
	QDomElement element = parent.ownerDocument().createElement("MIDIDeviceInterface");
	parent.appendChild(element);
	capturing.toXML(element);
	captureDevice.toXML(element);
}

MIDIDeviceInterface & MIDIDeviceInterface::fromXML(const QDomElement & parent)
{
	//try to find element in document
	QDomElement element = parent.firstChildElement("MIDIDeviceInterface");
	if (element.isNull())
	{
		throw std::runtime_error("No MIDI device settings found!");
	}
	//read device name from element
	capturing = false;
	captureDevice.fromXML(element);
	return *this;
}

void MIDIDeviceInterface::setCaptureDevice(const QString & inputName)
{
	if (m_midiWorker->captureDevice() != inputName)
	{
		QMetaObject::invokeMethod(m_midiWorker, "setCaptureDevice", Q_ARG(const QString &, inputName));
	}
}

void MIDIDeviceInterface::setCaptureState(bool capture)
{
	if (m_midiWorker->isCapturing() != capture)
	{
		QMetaObject::invokeMethod(m_midiWorker, "setCaptureState", Q_ARG(bool, capture));
	}
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
