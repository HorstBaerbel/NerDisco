#pragma once

#include "AudioConversion.h"
#include "AudioProcessing.h"

#include <QVector>
#include <QAudio>
#include <QAudioInput>
#include <QThread>


class AudioInterface: public QObject
{
	Q_OBJECT

public:
	AudioInterface(QObject *parent = 0);
	~AudioInterface();

	void setCurrentCaptureDevice(const QString & inputName);
	void setCaptureState(bool capturing);

	static QStringList inputDeviceNames();
	static QString defaultInputDeviceName();

	static QStringList ouputDeviceNames();
	static QString defaultOutputDeviceName();

signals:
	void captureDeviceChanged(const QString & name);
	void captureStateChanged(bool capturing);

	//Delivers audio levels for each channel.
	void levelData(const QVector<float> & levels, float timeus);
	//Delivers the FFT of the current audio data.
	void fftData(const QVector<float> & spectrum, int channels, float timeus);
	//Delivers beat information for the beat detection.
	void beatData(float bpm, bool isBeat);

protected slots:
	void inputDataReady();
	void inputStateChanged(QAudio::State state);

private:
	ConversionWorker * m_conversionWorker;
	ProcessingWorker * m_processingWorker;
	QThread m_workerThread;
	QAudioInput * m_audioInput;
	QIODevice * m_inputDevice;
	QString m_currentInputDeviceName;
	bool m_capturing;
	int m_captureInterval;
};
