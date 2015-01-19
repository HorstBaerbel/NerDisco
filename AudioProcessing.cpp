#include "AudioProcessing.h"


//-------------------------------------------------------------------------------------------------

ProcessingWorker::ProcessingWorker(QObject *parent)
	: QObject(parent)
	, m_doBeatDetection(false)
	, m_doFFT(true)
{
	qRegisterMetaType< QVector<float> >("QVector<float>");
}

void ProcessingWorker::processAudioData(const QVector<float> & data, float timeus)
{
	if (m_doFFT)
	{
	}
	if (m_doBeatDetection)
	{
	}
}

ProcessingWorker::~ProcessingWorker()
{
}

//-------------------------------------------------------------------------------------------------

AudioProcessing::AudioProcessing(QObject *parent)
	: QObject(parent)
	, m_worker(new ProcessingWorker())
{
	//register metatype so all signal/slot connections work
	qRegisterMetaType< QVector<float> >("QVector<float>");
	//do all possible connections to worker object
	connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(this, SIGNAL(processAudioData(const QVector<float> & data, float timeus)), m_worker, SLOT(processAudioData(const QVector<float> & data, float timeus)));
	//connect(m_worker, SIGNAL(audioDataProcessed(const QVector<float> &, float)), this, SIGNAL(audioDataCaptured(const QVector<float> &, float)));
	//move worker object to thread and run thread
	m_worker->moveToThread(&m_workerThread);
	m_workerThread.start();
}

AudioProcessing::~AudioProcessing()
{
	m_workerThread.quit();
	m_workerThread.wait();
}

void AudioProcessing::processAudioData(const QVector<float> & data, float timeus)
{
	//send data to worker thread for processing
	QMetaObject::invokeMethod(m_worker, "processAudioData", Q_ARG(const QVector<float> &, data), Q_ARG(float, timeus));
	//emit processAudioData(data, timeus);
}