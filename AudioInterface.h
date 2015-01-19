#pragma once

#include <QVector>
#include <QAudio>
#include <QAudioBuffer>
#include <QAudioInput>
#include <QThread>


//private worker class used in the interface
class InterfaceWorker : public QObject
{
	Q_OBJECT

public:
	InterfaceWorker(QObject *parent = 0);
	~InterfaceWorker();

signals:
	void audioDataProcessed(const QVector<float> & data, float timeus);

public slots:
	void processAudioData(const QByteArray & buffer, const QAudioFormat & format);
};

//-------------------------------------------------------------------------------------------------

class AudioInterface: public QObject
{
	Q_OBJECT

public:
	AudioInterface(QObject *parent = 0);
	~AudioInterface();

	QString currentInputDevice() const;
	void setCurrentInputDevice(const QString & inputName);

	void setCaptureState(bool capturing);

	static QStringList inputDeviceNames();
	static QString defaultInputDeviceName();

	static QStringList ouputDeviceNames();
	static QString defaultOutputDeviceName();

	//calculate audio duration based on data siuze and audio format
	static float getDuration(const QByteArray & buffer, const QAudioFormat & format);
	//Returns the maximum possible sample value for a given audio format
	static float getPeakValue(const QAudioFormat & format);
	//Get maximum levels for each channel
	static QVector<float> getBufferLevels(const QByteArray & buffer, const QAudioFormat & format);

signals:
	void inputDeviceChanged(const QString & name);
	void captureStateChanged(bool capturing);
	void audioDataCaptured(const QVector<float> & data, float timeus);

protected slots:
	void inputDataReady();
	void inputStateChanged(QAudio::State state);

private:
	InterfaceWorker * m_worker;
	QThread m_workerThread;
	QAudioInput * m_audioInput;
	QIODevice * m_inputDevice;
	QString m_currentInputDeviceName;
	bool m_capturing;
	int m_captureInterval;
};
