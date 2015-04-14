#pragma once

#include <QObject>
#include <QVector>


//worker class used in the interface
class ProcessingWorker : public QObject
{
	Q_OBJECT

public:
	ProcessingWorker(QObject *parent = 0);
	~ProcessingWorker();

	void enableLevelsData(bool enable = true);
	void enableBeatData(bool enable = false);
	void enableFFTData(bool enable = false);

signals:
	//Delivers audio levels for each channel.
	void levelData(const QVector<float> & levels, float timeus);
	//Delivers the FFT of the current audio data.
	void fftData(const QVector<float> & spectrum, int channels, float timeus);
	//Delivers beat information for the beat detection.
	void beatData(float bpm, bool isBeat);

	void output(const QVector<float> & data, int channels, float timeus);

public slots:
	void input(const QVector<float> & data, int channels, float timeus);

private:
	//Get maximum levels for each channel
	QVector<float> getMaximumLevels(const QVector<float> & data, int channels);
	//Do the FFT for a bit of audio data
	QVector<float> getSpectrum(const QVector<float> & data, int channels);

	bool m_doFFT;
	bool m_doBeatDetection;
	bool m_doLevels;
};
