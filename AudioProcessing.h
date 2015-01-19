#pragma once

#include <QVector>
#include <QObject>
#include <QThread>

//#include "kiss_fft\tools\kiss_fftr.h"


//private worker class used in the interface
class ProcessingWorker : public QObject
{
	Q_OBJECT

public:
	ProcessingWorker(QObject *parent = 0);
	~ProcessingWorker();

	void enableBeatDetection(bool enable = false);
	void enableFFT(bool enable = true);

signals:
	void fftResult(const QVector<float> & spectrum, float timeus);
	void beatResult(float bpm, bool isBeat);

public slots:
	void processAudioData(const QVector<float> & data, float timeus);

private:
	bool m_doFFT;
	bool m_doBeatDetection;
};

class AudioProcessing : public QObject
{
	Q_OBJECT

public:
	AudioProcessing(QObject *parent);
	~AudioProcessing();
	
signals:

public slots:
	void processAudioData(const QVector<float> & data, float timeus);

private:
	ProcessingWorker * m_worker;
	QThread m_workerThread;
};
