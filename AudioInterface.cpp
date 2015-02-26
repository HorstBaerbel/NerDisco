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
	, m_capturing(false)
	, m_captureInterval(33)
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

void AudioInterface::inputStateChanged(QAudio::State state)
{
	emit captureStateChanged(state == QAudio::ActiveState /* || state == QAudio::IdleState*/);
}

void AudioInterface::setCaptureState(bool capturing)
{
	//check if the capture state changed
	if (m_capturing != capturing)
	{
		m_capturing = capturing;
		//check if we want to start or stop capturing
		if (m_capturing)
		{
			if (m_audioInput)
			{
				//create buffer receiving data
				m_inputDevice = new QBuffer(this);
				m_inputDevice->open(QIODevice::ReadWrite);
				//connect(m_inputDevice, SIGNAL(readyRead()), this, SLOT(inputDataReady()));
				//allocate audio buffer sized twice the capture interval and start input
				m_audioInput->setBufferSize(m_audioInput->format().bytesForDuration(1000 * 2 * m_captureInterval));
				m_audioInput->start(m_inputDevice);
				//m_inputDevice = m_audioInput->start(); //DOES NOT WORK! Why. ever.
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
			}
		}
	}
}

void AudioInterface::setCurrentCaptureDevice(const QString & inputName)
{
	//check if the audio device changed
	if (m_currentInputDeviceName != inputName)
	{
		m_currentInputDeviceName = inputName;
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
		}
		//if we've got a device name, try to find the device
		if (!m_currentInputDeviceName.isEmpty())
		{
			//find device for name
            foreach (const QAudioDeviceInfo & info, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
			{
				if (info.deviceName() == m_currentInputDeviceName)
				{
					//device found. create capture format
					QAudioFormat format;
					format.setSampleRate(44100);
					format.setChannelCount(1);
					format.setSampleSize(8);
					format.setCodec("audio/pcm");
					format.setByteOrder(QAudioFormat::LittleEndian);
					format.setSampleType(QAudioFormat::UnSignedInt);
					if (!info.isFormatSupported(format))
					{
						//formast not supported, try sonething similar
						format = info.nearestFormat(format);
					}
					//create audio input
					m_audioInput = new QAudioInput(info, format, this);
					m_audioInput->setNotifyInterval(m_captureInterval);
					connect(m_audioInput, SIGNAL(notify()), this, SLOT(inputDataReady()));
					connect(m_audioInput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(inputStateChanged(QAudio::State)));
					break;
				}
			}
		}
		emit captureDeviceChanged(m_currentInputDeviceName);
	}
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
