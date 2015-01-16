#include "AudioInterface.h"

#include <QAudioDeviceInfo>
#include <QBuffer>
#include <QDebug>

//-------------------------------------------------------------------------------------------------

AudioWorker::AudioWorker(QObject *parent)
	: QObject(parent)
{
	qRegisterMetaType<QVector<float>>("QVector<float>");
}

void AudioWorker::processAudioData(const QByteArray & buffer, const QAudioFormat & format)
{
	QVector<float> levels = AudioInterface::getBufferLevels(buffer, format);
	emit audioDataProcessed(levels, AudioInterface::getDuration(buffer, format));
}

AudioWorker::~AudioWorker()
{
}

//-------------------------------------------------------------------------------------------------

AudioInterface::AudioInterface(QObject *parent)
	: QObject(parent)
	, m_worker(new AudioWorker())
	, m_audioInput(nullptr)
	, m_inputDevice(nullptr)
	, m_capturing(false)
	, m_captureInterval(33)
{
	//register metatype so all signal/slot connections work
	qRegisterMetaType<QVector<float>>("QVector<float>");
	//do all possible connections to worker object
	connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(this, SIGNAL(processAudioData(const QByteArray &, const QAudioFormat &)), m_worker, SLOT(processAudioData(const QByteArray &, const QAudioFormat &)));
	connect(m_worker, SIGNAL(audioDataProcessed(const QVector<float> &, float)), this, SIGNAL(audioDataCaptured(const QVector<float> &, float)));
	//move worker object to thread and run thread
	m_worker->moveToThread(&m_workerThread);
	m_workerThread.start();
}

AudioInterface::~AudioInterface()
{
	m_audioInput->stop();
	delete m_audioInput;
	delete m_inputDevice;
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
		qDebug() << "AudioInterface capture state changed";
		m_capturing = capturing;
		//check if we want to start or stop capturing
		if (m_capturing)
		{
			if (m_audioInput)
			{
				qDebug() << "Starting capture";
				//create buffer receiving data
				m_inputDevice = new QBuffer(this);
				m_inputDevice->open(QIODevice::ReadWrite);
				//connect(m_inputDevice, SIGNAL(readyRead()), this, SLOT(inputDataReady()));
				//allocate buffer for 100ms of audio and start input
				m_audioInput->setBufferSize(m_audioInput->format().bytesForDuration(1000 * 2 * m_captureInterval));
				m_audioInput->start(m_inputDevice);
				//m_inputDevice = m_audioInput->start(m_inputDevice); //DOES NOT WORK!
			}
		}
		else
		{
			if (m_audioInput)
			{
				qDebug() << "Stopping capture";
				m_audioInput->stop();
				m_inputDevice->close();
				m_inputDevice->disconnect(this);
				delete m_inputDevice;
				m_inputDevice = nullptr;
			}
		}
	}
}

void AudioInterface::setCurrentInputDevice(const QString & inputName)
{
	//check if the audio device changed
	if (m_currentInputDeviceName != inputName)
	{
		qDebug() << "AudioInterface device name change";
		m_currentInputDeviceName = inputName;
		//if the current device is running, stop it
		if (m_audioInput && m_audioInput->state() == QAudio::ActiveState)
		{
			m_audioInput->stop();
			m_audioInput->disconnect(this);
			delete m_audioInput;
			m_audioInput = nullptr;
			m_inputDevice->disconnect(this);
			delete m_inputDevice;
			m_inputDevice = nullptr;
		}
		//if we've got a device name, try to find the device
		if (!m_currentInputDeviceName.isEmpty())
		{
			//find device for name
			for (const QAudioDeviceInfo & info : QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
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
		emit inputDeviceChanged(m_currentInputDeviceName);
	}
}

QString AudioInterface::currentInputDevice() const
{
	return m_currentInputDeviceName;
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

float AudioInterface::getDuration(const QByteArray & buffer, const QAudioFormat & format)
{
	if (!format.isValid())
		return 0.0f;
	if (format.codec() != "audio/pcm")
		return 0.0f;
	if (buffer.size() <= 0)
		return 0.0f;
	return (float)(format.durationForBytes(buffer.size()));
}

float AudioInterface::getPeakValue(const QAudioFormat& format)
{
	// Note: Only the most common sample formats are supported
	if (!format.isValid())
		return 0.0f;
	if (format.codec() != "audio/pcm")
		return 0.0f;

	switch (format.sampleType()) {
	case QAudioFormat::Unknown:
		break;
	case QAudioFormat::Float:
		if (format.sampleSize() != 32) // other sample formats are not supported
			return qreal(0);
		return 1.00003f;
	case QAudioFormat::SignedInt:
		if (format.sampleSize() == 32)
			return (float)INT_MAX;
		if (format.sampleSize() == 16)
			return (float)SHRT_MAX;
		if (format.sampleSize() == 8)
			return (float)CHAR_MAX;
		break;
	case QAudioFormat::UnSignedInt:
		if (format.sampleSize() == 32)
			return (float)UINT_MAX;
		if (format.sampleSize() == 16)
			return (float)USHRT_MAX;
		if (format.sampleSize() == 8)
			return (float)UCHAR_MAX;
		break;
	}
	return 0.0f;
}

template <class T>
QVector<float> getLevels(const T *buffer, int frames, int channels)
{
	QVector<float> max_values;
	max_values.fill(0.0f, channels);

	for (int i = 0; i < frames; ++i) {
		for (int j = 0; j < channels; ++j) {
			qreal value = qAbs((float)(buffer[i * channels + j]));
			if (value > max_values.at(j))
				max_values.replace(j, value);
		}
	}
	return max_values;
}

QVector<float> AudioInterface::getBufferLevels(const QByteArray & buffer, const QAudioFormat & format)
{
	QVector<float> values;

	if (!format.isValid() || format.byteOrder() != QAudioFormat::LittleEndian)
		return values;
	if (format.codec() != "audio/pcm")
		return values;
	if (buffer.size() <= 0)
		return values;

	const int frames = format.framesForBytes(buffer.size());
	const int channelCount = format.channelCount();
	values.fill(0.0f, channelCount);
	float peak_value = getPeakValue(format);
	if (qFuzzyCompare(peak_value, 0.0f))
		return values;

	switch (format.sampleType()) {
	case QAudioFormat::Unknown:
	case QAudioFormat::UnSignedInt:
		if (format.sampleSize() == 32)
			values = getLevels((quint32*)buffer.constData(), frames, channelCount);
		if (format.sampleSize() == 16)
			values = getLevels((quint16*)buffer.constData(), frames, channelCount);
		if (format.sampleSize() == 8)
			values = getLevels((quint8*)buffer.constData(), frames, channelCount);
		for (int i = 0; i < values.size(); ++i)
			values[i] = qAbs(values.at(i) - 0.5f * peak_value) / (0.5f * peak_value);
		break;
	case QAudioFormat::Float:
		if (format.sampleSize() == 32) {
			values = getLevels((float*)buffer.constData(), frames, channelCount);
			for (int i = 0; i < values.size(); ++i)
				values[i] /= peak_value;
		}
		break;
	case QAudioFormat::SignedInt:
		if (format.sampleSize() == 32)
			values = getLevels((qint32*)buffer.constData(), frames, channelCount);
		if (format.sampleSize() == 16)
			values = getLevels((qint16*)buffer.constData(), frames, channelCount);
		if (format.sampleSize() == 8)
			values = getLevels((qint8*)buffer.constData(), frames, channelCount);
		for (int i = 0; i < values.size(); ++i)
			values[i] /= peak_value;
		break;
	}
	return values;
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
			emit processAudioData(m_inputDevice->readAll(), m_audioInput->format());
		}
		m_inputDevice->reset();
	}
}
