#include "AudioProcessing.h"
//#include "kiss_fft\tools\kiss_fftr.h"


ProcessingWorker::ProcessingWorker(QObject *parent)
	: QObject(parent)
	, m_doLevels(true)
	, m_doBeatDetection(false)
	, m_doFFT(false)
{
	qRegisterMetaType< QVector<float> >("QVector<float>");
}

void ProcessingWorker::enableLevelsData(bool enable)
{
	m_doLevels = enable;
}

void ProcessingWorker::enableBeatData(bool enable)
{
	m_doBeatDetection = enable;
}

void ProcessingWorker::enableFFTData(bool enable)
{
	m_doFFT = enable;
}

void ProcessingWorker::input(const QVector<float> & data, int channels, float timeus)
{
	if (m_doLevels)
	{
		QVector<float> levels = getMaximumLevels(data, channels);
		emit levelData(levels, timeus);
	}
	if (m_doFFT)
	{
		QVector<float> processed;
		if (m_doBeatDetection)
		{
		}
	}
}

QVector<float> ProcessingWorker::getMaximumLevels(const QVector<float> & data, int channels)
{
	QVector<float> maxLevels(channels, 0.0f);
	const int frames = data.size() / channels;
	for (int i = 0; i < frames; ++i) {
		for (int j = 0; j < channels; ++j) {
			qreal value = qAbs(data[j]);
			maxLevels[j] = value > maxLevels[j] ? value : maxLevels[j];
		}
	}
	return maxLevels;
}

QVector<float> ProcessingWorker::getSpectrum(const QVector<float> & /*data*/, int /*channels*/)
{

	return QVector<float>(1, 0.0f);
}

ProcessingWorker::~ProcessingWorker()
{
}
