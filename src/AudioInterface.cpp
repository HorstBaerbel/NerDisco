#include "AudioInterface.h"

#include <QAudioDeviceInfo>
#include <QBuffer>
#include <QDebug>


AudioInterface::AudioInterface(QObject *parent)
	: QObject(parent)
	, m_conversionWorker(new ConversionWorker())
	, m_processingWorker(new ProcessingWorker())
    , m_audioInput(NULL)
    , m_inputDevice(NULL)
	, captureDevice("captureDevice", "")
	, capturing("capturing", false)
	, captureInterval("captureInterval", 20, 10, 50)
{
	//register metatype so all signal/slot connections work
    qRegisterMetaType< QVector<float> >("QVector<float>");
	//do all possible connections to worker objects
	connect(&m_workerThread, &QThread::finished, m_conversionWorker, &QObject::deleteLater);
	connect(&m_workerThread, &QThread::finished, m_processingWorker, &QObject::deleteLater);
	//build pseudo filter pipe
	connect(m_conversionWorker, SIGNAL(output(const QVector<float> &, int, float)), m_processingWorker, SLOT(input(const QVector<float> &, int, float)));
	//connect returning signals
	connect(m_processingWorker, SIGNAL(levelData(const QVector<float> &, float)), this, SIGNAL(levelData(const QVector<float> &, float)));
	connect(m_processingWorker, SIGNAL(fftData(const QVector<float> &, int, float)), this, SIGNAL(fftData(const QVector<float> &, int, float)));
	connect(m_processingWorker, SIGNAL(beatData(float, bool)), this, SIGNAL(beatData(float, bool)));
	//connect parameters to internal slots
	connect(captureDevice.GetSharedParameter().get(), SIGNAL(valueChanged(const QString &)), this, SLOT(setCaptureDevice(const QString &)));
	connect(capturing.GetSharedParameter().get(), SIGNAL(valueChanged(bool)), this, SLOT(setCaptureState(bool)));
	connect(captureInterval.GetSharedParameter().get(), SIGNAL(valueChanged(int)), this, SLOT(setCaptureInterval(int)));
	//move worker objects to thread and run thread
	m_conversionWorker->moveToThread(&m_workerThread);
	m_processingWorker->moveToThread(&m_workerThread);
	m_workerThread.start();
}

AudioInterface::~AudioInterface()
{
    if (m_audioInput)
    {
        m_audioInput->stop();
        delete m_audioInput;
        delete m_inputDevice;
    }
	m_workerThread.quit();
	m_workerThread.wait();
}

void AudioInterface::toXML(QDomElement & parent) const
{
	//try to find element in document
	QDomElement element = parent.firstChildElement("AudioInterface");
	if (element.isNull())
	{
		//add the new element
		element = parent.ownerDocument().createElement("AudioInterface");
		parent.appendChild(element);
	}
	captureDevice.toXML(element);
	captureInterval.toXML(element);
}

AudioInterface & AudioInterface::fromXML(const QDomElement & parent)
{
	//try to find element in document
	QDomElement element = parent.firstChildElement("AudioInterface");
	if (element.isNull())
	{
		throw std::runtime_error("No audio device settings found!");
	}
	//read device name from element
	capturing = false;
	captureDevice.fromXML(element);
	captureInterval.fromXML(element);
	return *this;
}

void AudioInterface::inputStateChanged(QAudio::State state)
{
	capturing = (state == QAudio::ActiveState /* || state == QAudio::IdleState*/);
}

void AudioInterface::setCaptureState(bool on)
{
	//check if we want to start or stop capturing
	if (on)
	{
		if (m_audioInput)
		{
			//set up processing worker with current sample rate and bit depth
			m_processingWorker->setSampleRate(m_sampleRate);
			m_processingWorker->setBitDepth(m_bitDepth);
			//create buffer receiving data
			m_inputDevice = new QBuffer(this);
			m_inputDevice->open(QIODevice::ReadWrite);
			//connect(m_inputDevice, SIGNAL(readyRead()), this, SLOT(inputDataReady()));
			m_audioInput->start(m_inputDevice);
			//m_inputDevice = m_audioInput->start(); //DOES NOT WORK! Why. ever.
		}
		else
		{
			capturing = false;
		}
	}
	else
	{
		if (m_audioInput)
		{
			m_audioInput->stop();
			m_inputDevice->close();
			m_inputDevice->disconnect(this);
			delete m_inputDevice;
			m_inputDevice = NULL;
			capturing = false;
		}
	}
}

void AudioInterface::setCaptureDevice(const QString & inputName)
{
	//if the current device is running, stop it
	if (m_audioInput && m_audioInput->state() == QAudio::ActiveState)
	{
		m_audioInput->stop();
		m_audioInput->disconnect(this);
		delete m_audioInput;
		m_audioInput = NULL;
		m_inputDevice->disconnect(this);
		delete m_inputDevice;
		m_inputDevice = NULL;
		capturing = false;
	}
	//if we've got a device name, try to find the device
	if (!inputName.isEmpty())
	{
		//find device for name
		foreach(const QAudioDeviceInfo & info, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
		{
			if (info.deviceName() == inputName)
			{
				//device found. create capture format
				QAudioFormat format;
				format.setSampleRate(m_sampleRate);
				format.setChannelCount(1);
				format.setSampleSize(m_bitDepth);
				format.setCodec("audio/pcm");
				format.setByteOrder(QAudioFormat::LittleEndian);
				format.setSampleType(QAudioFormat::UnSignedInt);
				if (!info.isFormatSupported(format))
				{
					//format not supported, try something similar
					format = info.nearestFormat(format);
				}
				//create audio input
				m_audioInput = new QAudioInput(info, format, this);
				m_audioInput->setNotifyInterval(captureInterval);
				//allocate audio buffer sized twice the capture interval
				m_audioInput->setBufferSize(m_audioInput->format().bytesForDuration(1000 * 2 * captureInterval));
				connect(m_audioInput, SIGNAL(notify()), this, SLOT(inputDataReady()));
				connect(m_audioInput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(inputStateChanged(QAudio::State)));
				captureDevice = inputName;
				break;
			}
		}
	}
	else
	{
		captureDevice = "";
	}
}

void AudioInterface::setCaptureInterval(int interval)
{
	if (m_audioInput && m_inputDevice)
	{
		const bool inputActive = m_audioInput && m_audioInput->state() == QAudio::ActiveState;
		if (inputActive)
		{
			//if capturing stop input
			m_audioInput->stop();
			m_inputDevice->close();
		}
		//change callback interval and buffer size
		m_audioInput->setNotifyInterval(interval);
		m_audioInput->setBufferSize(m_audioInput->format().bytesForDuration(1000 * 2 * interval));
		if (inputActive)
		{
			//if capturing, restart input
			m_inputDevice->open(QIODevice::ReadWrite);
			m_audioInput->start(m_inputDevice);
		}
	}
	else if (m_audioInput)
	{
		//change callback interval and buffer size
		m_audioInput->setNotifyInterval(interval);
		m_audioInput->setBufferSize(m_audioInput->format().bytesForDuration(1000 * 2 * interval));
	}
	captureInterval = interval;
}

QStringList AudioInterface::inputDeviceNames()
{
	QStringList deviceNames;
	foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
	{
		deviceNames.append(deviceInfo.deviceName());
	}
	return deviceNames;
}

QString AudioInterface::defaultInputDeviceName()
{
	return QAudioDeviceInfo::defaultInputDevice().deviceName();
}

QStringList AudioInterface::ouputDeviceNames()
{
	QStringList deviceNames;
	foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
	{
		deviceNames.append(deviceInfo.deviceName());
	}
	return deviceNames;
}

QString AudioInterface::defaultOutputDeviceName()
{
	return QAudioDeviceInfo::defaultOutputDevice().deviceName();
}

void AudioInterface::inputDataReady()
{
	if (m_inputDevice)
	{
		//seek to start of buffer
		m_inputDevice->seek(0);
		//check if we have data to process
		if (m_inputDevice->bytesAvailable() > 0)
		{
			//send data to worker thread for processing
			QMetaObject::invokeMethod(m_conversionWorker, "input", Q_ARG(const QByteArray &, m_inputDevice->readAll()), Q_ARG(const QAudioFormat &, m_audioInput->format()));
			//emit output(m_inputDevice->readAll(), m_audioInput->format());
		}
		m_inputDevice->reset();
	}
}
