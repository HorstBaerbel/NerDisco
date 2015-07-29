#pragma once

#include "AudioConversion.h"
#include "AudioProcessing.h"
#include "Parameters.h"

#include <QVector>
#include <QAudio>
#include <QAudioInput>
#include <QThread>
#include <QDomDocument>


class AudioInterface: public QObject
{
	Q_OBJECT

public:
	AudioInterface(QObject *parent = 0);
	~AudioInterface();

	/// @brief Save the current settings to an XML document.
	/// @param parent The paren element to write the settings to.
	void toXML(QDomElement & parent) const;
	/// @brief Read current settings from XML document.
	/// @param parent The parent element to load the settings from.
	AudioInterface & fromXML(const QDomElement & parent);

	ParameterQString captureDevice;
	ParameterBool capturing;
	ParameterInt captureInterval;

	static QStringList inputDeviceNames();
	static QString defaultInputDeviceName();

	static QStringList ouputDeviceNames();
	static QString defaultOutputDeviceName();

signals:
	//Delivers audio levels for each channel.
	void levelData(const QVector<float> & levels, float timeus);
	//Delivers the FFT of the current audio data.
	void fftData(const QVector<float> & spectrum, int channels, float timeus);
	//Delivers beat information for the beat detection.
	void beatData(float bpm, bool isBeat);

protected slots:
	void setCaptureDevice(const QString & inputName);
	void setCaptureState(bool capturing);
	void setCaptureInterval(int interval);

	void inputDataReady();
	void inputStateChanged(QAudio::State state);

private:
	ConversionWorker * m_conversionWorker = nullptr;
	ProcessingWorker * m_processingWorker = nullptr;
	QThread m_workerThread;
	QAudioInput * m_audioInput = nullptr;
	QIODevice * m_inputDevice = nullptr;
	int m_sampleRate = 44100;
	int m_bitDepth = 16;
};
