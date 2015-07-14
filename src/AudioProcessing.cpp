#include "AudioProcessing.h"

#ifdef USE_KISSFFT
	#include "../kiss_fft/kiss_fft.h"
	#include "../kiss_fft/tools/kiss_fftr.h"
#endif



ProcessingWorker::ProcessingWorker(int sampleRate, QObject *parent)
	: QObject(parent)
	, m_doLevels(true)
	, m_doBeatDetection(false)
	, m_doFFT(false)
{
	qRegisterMetaType< QVector<float> >("QVector<float>");
	setSampleRate(sampleRate);
}

ProcessingWorker::~ProcessingWorker()
{
#ifdef USE_KISSFFT
	if (m_fftConfig)
	{
		kiss_fftr_free(m_fftConfig);
	}
	delete[] m_fftResult;
#endif
}

void ProcessingWorker::setSampleRate(int sampleRate)
{
	if (m_sampleRate != sampleRate)
	{
		m_sampleRate = sampleRate;
		//set FFT window size regarding to sample rate
		if (m_sampleRate <= 11025)
		{
			m_fftWindowSize = 1024;
		}
		else if (m_sampleRate >= 22050 || m_sampleRate < 44100)
		{
			m_fftWindowSize = 2048;
		}
		else
		{
			m_fftWindowSize = 4096;
		}
		m_fftBinSize = m_fftWindowSize / 2 + 1;
#ifdef USE_KISSFFT
		//check if we need to re-allocate fft memory
		if (!m_fftConfig || m_fftConfig->substate->nfft != m_fftWindowSize)
		{
			//allocate KissFFT configuration structure
			if (m_fftConfig)
			{
				kiss_fftr_free(m_fftConfig);
			}
			m_fftConfig = kiss_fftr_alloc(m_fftWindowSize, 0, NULL, NULL);
			//allocate memory for results
			m_fftBinSize = m_fftWindowSize / 2 + 1;
			delete[] m_fftResult;
			m_fftResult = new kiss_fft_cpx[m_fftBinSize];
		}
#endif
	}
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
#ifdef USE_KISSFFT
	if (m_doFFT)
	{
		bool isBeat = false;
		QVector<float> processed = doFFT(data.constData(), channels);
		if (m_doBeatDetection)
		{
			//reduce spectrum to half the channels and calculate average volume of low frequencies
			float avgVol = 0.0;
			QVector<float> reduced;
			for (int i = 0; i < processed.size() / 2; i+=2)
			{
				const float value = processed.at(i) + processed.at(i + 1);
				avgVol += value;
				reduced.append(value);
			}
			//find peaks (volume > avgVolume * 1.5)
			QVector<int> peaks;
			for (int i = 0; i < reduced.size() / 4; ++i)
			{
				if (reduced.at(i) > (avgVol * 1.5f))
				{
					peaks.append(i);
				}
			}
			//insert peak indices into queue

			//TODO

			//check if beat (peak in the same position for consecutive frames)
			for (int i = 0; i < peaks.size(); ++i)
			{
				bool beat = true;
				//search through peak list in reverse order
				auto pIt = m_previousPeakPositions.cbegin();
				for (int j = m_previousPeakPositions.size() - 1; j >= 0; --j)
				{
					if (!m_previousPeakPositions.at(j).contains(i))
					{
						beat = false;
						break;
					}
				}
				//if we have a beat here, return
				if (beat)
				{
					isBeat = true;
					break;
				}
			}
		}
	}
#endif
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

#ifdef USE_KISSFFT
	QVector<float> ProcessingWorker::doFFT(const float * data, int channels)
	{
		//apply fft to data
		kiss_fftr(m_fftConfig, data, m_fftResult);
		//copy real result to vector
		QVector<float> result(m_fftBinSize);
		for (int i = 0; i < m_fftBinSize; ++i)
		{
			result[i] = m_fftResult[i].r;
		}
		return result;
	}
#endif
