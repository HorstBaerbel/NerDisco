#include "AudioProcessing.h"

#include "../kiss_fft/kiss_fft.h"
#include "../kiss_fft/tools/kiss_fftr.h"

#include <QDebug>
#include <math.h>

static QVector<float> debugSignal;
static int debugSignalSize = 0;


ProcessingWorker::ProcessingWorker(int sampleRate, int bitDepth, QObject *parent)
	: QObject(parent)
{
	qRegisterMetaType< QVector<float> >("QVector<float>");
	setSampleRate(sampleRate);
	setBitDepth(bitDepth);
}

ProcessingWorker::~ProcessingWorker()
{
	if (m_fftConfig)
	{
		kiss_fftr_free(m_fftConfig);
	}
	delete[] m_fftResult;
	delete[] m_intermediateBuffer;
	delete[] m_windowFunctionCoefficients;
}

void ProcessingWorker::UpdateWindowCoefficients()
{
	delete[] m_windowFunctionCoefficients;
	m_windowFunctionCoefficients = new float[m_fftWindowSize];
	//Hanning window: 0.5 * (1 - cos(2*pi*n)/(N-1))
	m_windowFunctionCoefficientSum = 0.0f;
	for (int i = 0; i < m_fftWindowSize; ++i) {
		m_windowFunctionCoefficients[i] = 0.5f * (1.0f - std::cos((2.0f * 3.1415926f) * i / (m_fftWindowSize - 1)));
		m_windowFunctionCoefficientSum += m_windowFunctionCoefficients[i];
	}
}

void ProcessingWorker::UpdateKissConfig()
{
	if (m_kissConfigChanged)
	{
		//set FFT window size regarding to sample rate
		if (m_sampleRate <= 11025)
		{
			m_fftWindowSize = 1024;
		}
		else if (m_sampleRate >= 22050 && m_sampleRate < 44100)
		{
			m_fftWindowSize = 2048;
		}
		else
		{
			m_fftWindowSize = 2048;
		}
		//calcuate size of real-only FFT bins. This includes DC in index 0 and the Nyquist frequency in index m_fftBinSize
		m_fftBinSize = m_fftWindowSize / 2 + 1;
		m_fftWindowOverlap = m_fftWindowSize / 16;
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
			delete[] m_fftResult;
			m_fftResult = new kiss_fft_cpx[m_fftBinSize];
			delete[] m_intermediateBuffer;
			m_intermediateBuffer = new float[m_fftWindowSize];
			m_spectrum.resize(m_fftBinSize);
			//calculate new window coefficients
			UpdateWindowCoefficients();
			//build debug periodic signal
			debugSignal.resize(m_fftWindowSize * 1.5f);
			for (int i = 0; i < debugSignal.size(); ++i)
			{
				float p = i / (m_sampleRate / 4000.0f);
				float value = std::sin(2.0f * 3.1415926f * p);
				debugSignal[i] = value;
			}
		}
		m_kissConfigChanged = false;
	}
}

void ProcessingWorker::setSampleRate(int sampleRate)
{
	if (m_sampleRate != sampleRate)
	{
		m_sampleRate = sampleRate;
		m_kissConfigChanged = true;
	}
}

void ProcessingWorker::setBitDepth(int bitDepth)
{
	if (m_bitDepth != bitDepth)
	{
		m_bitDepth = bitDepth;
		m_Sqnr = 20.0f * std::log10(std::pow(2.0f, m_bitDepth));
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
	if (m_doFFT)
	{
		bool isBeat = false;
		//update KissFFT config if necessary
		UpdateKissConfig();
		//clear spectrum data
		m_spectrum.fill(0.0f);
		float * spectrumData = m_spectrum.data();
		//we might have data left from last frame, append the new data to it
		m_data += debugSignal.mid(0, data.size());//data;
		const int nrOfSamples = m_data.size(); //# of samples left to process
		//check if we have sufficient samples to do an FFT
		if (nrOfSamples >= m_fftWindowSize)
		{
			//process data in m_fftWindowSize sized chunks
			int nrOfDataChunks = 0; //how many chunks of m_fftWindowSize we've processed
			int startSample = 0; //current processing sample start index
			const float * srcData = m_data.constData();
			//loop while there are enough samples left until the end of the data
			while ((startSample + m_fftWindowSize) <= nrOfSamples)
			{
				//first apply window function to signal
				//applyWindowFunction(m_intermediateBuffer, &srcData[startSample], m_windowFunctionCoefficients, m_fftWindowSize, channels);
				//apply fft to data
				kiss_fftr(m_fftConfig, &srcData[startSample], m_fftResult);
				//apply normalization to complex values
				//normalizeFFTResult(m_fftResult, m_fftBinSize, m_fftWindowSize, m_windowFunctionCoefficientSum);
				//calculate magnitude from result
				calculateSquareMagnitude(m_intermediateBuffer, m_fftResult, m_fftBinSize);
				//convert square magnitude to dB scale
				calculateMagnitudedB(m_intermediateBuffer, m_intermediateBuffer, m_fftBinSize);
				//add data to final spectrum result
				for (int i = 0; i < m_fftBinSize; ++i)
				{
					spectrumData[i] += m_intermediateBuffer[i];
				}
				//step to start of next window minus overlap
				startSample += m_fftWindowSize - m_fftWindowOverlap;
				nrOfDataChunks++;
			}
			//divide result by number of chunks
			for (int i = 0; i < m_fftBinSize; ++i)
			{
				spectrumData[i] /= nrOfDataChunks;
			}
			//store remaining samples for next frame, remove the ones we've processed already
			m_data.remove(0, startSample);
			//m_data.clear();
			//average spectrum into full octave bands
			QVector<float> octaveBands = calculateOctaveBands(spectrumData, m_fftBinSize, m_fftWindowSize, m_sampleRate);
			//QVector<float> octaveBands = averageBands(spectrumData, m_fftBinSize, 32);
			//normalize the values by dividing by the SQNR value for the signal bit depth
			normalizeValuesSQNR(octaveBands.data(), octaveBands.constData(), octaveBands.size(), m_Sqnr);
			//qDebug() << octaveBands;
			emit fftData(octaveBands, channels, timeus);
			//if (m_doBeatDetection)
			//{
			//	//reduce spectrum to half the channels and calculate average volume of low frequencies
			//	float avgVol = 0.0;
			//	QVector<float> reduced;
			//	for (int i = 0; i < m_spectrum.size() / 2; i += 2)
			//	{
			//		const float value = m_spectrum.at(i) + m_spectrum.at(i + 1);
			//		avgVol += value;
			//		reduced.append(value);
			//	}
			//	//find peaks (volume > avgVolume * 1.5)
			//	QVector<int> peaks;
			//	for (int i = 0; i < reduced.size() / 4; ++i)
			//	{
			//		if (reduced.at(i) > (avgVol * 1.5f))
			//		{
			//			peaks.append(i);
			//		}
			//	}
			//	//insert peak indices into queue

			//	//TODO

			//	//check if beat (peak in the same position for consecutive frames)
			//	for (int i = 0; i < peaks.size(); ++i)
			//	{
			//		bool beat = true;
			//		//search through peak list in reverse order
			//		auto pIt = m_previousPeakPositions.cbegin();
			//		for (int j = m_previousPeakPositions.size() - 1; j >= 0; --j)
			//		{
			//			if (!m_previousPeakPositions.at(j).contains(i))
			//			{
			//				beat = false;
			//				break;
			//			}
			//		}
			//		//if we have a beat here, return
			//		if (beat)
			//		{
			//			isBeat = true;
			//			break;
			//		}
			//	}
			//}
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

void ProcessingWorker::applyWindowFunction(float * dest, const float * src, const float * windowData, const int nrOfSamples, int channels)
{
	//apply window funtion to data
	for (int i = 0; i < nrOfSamples; ++i)
	{
		dest[i] = src[i] * windowData[i];
	}
}

void ProcessingWorker::normalizeFFTResult(kiss_fft_cpx * complex, const int fftBinSize, const int fftWindowSize, const float windowFunctionCoefficientSum)
{
	//FFT results should be normalized using FFT size, also the FFT bin size is only half of the
	//window size, thus we scale all value by 2. furthermore we need to accound for the window
	//function and thus multiply by the sum of its coefficients.
	const float normalizationValue = 2.0f / windowFunctionCoefficientSum / (float)fftWindowSize;
	for (int i = 0; i < fftBinSize; ++i)
	{
		complex[i].r *= normalizationValue;
		complex[i].i *= normalizationValue;
	}
}

void ProcessingWorker::calculateSquareMagnitude(float * dest, const kiss_fft_cpx * complexResult, const int fftBinSize)
{
	//calculate square, absolute value of complex result, scale by 2 to conserve total energy
	for (int i = 0; i < fftBinSize; ++i)
	{
		dest[i] = complexResult[i].r * complexResult[i].r + complexResult[i].i * complexResult[i].i;
	}
}

void ProcessingWorker::calculateMagnitudedB(float * dest, const float * src, const int fftBinSize)
{
	//copy real result to destination
	for (int i = 0; i < fftBinSize; ++i)
	{
		dest[i] = 10.0f * std::log10(src[i]);
	}
}

QVector<float> ProcessingWorker::averageBands(const float * src, const int fftBinSize, const int factor)
{
	const int resultBands = (fftBinSize - 1) / factor;
	QVector<float> result(resultBands, 0.0f);
	//loop through all bins, averaging the values
	int resultIndex = 0;
	for (int i = 1; i < (fftBinSize - 1);)
	{
		for (int j = 0; j < factor; ++j, ++i)
		{
			result.data()[resultIndex] += src[i];
		}
		result.data()[resultIndex] /= factor;
		resultIndex++;
	}
	return result;
}

QVector<float> ProcessingWorker::calculateOctaveBands(const float * src, const int fftBinSize, const int windowSize, const int sampleRate)
{
	//calculate the frequency range of a spectrum bin, depending on the samplerate and 
	//the amount of samples taken (aka the window size), e.g. 44100 / 1024 = ~43Hz per bin.
	const float frequencyRangePerBin = (float)sampleRate / (float)windowSize;
	//The frequency range from 0->20 kHz can be split into 11 octave band, 21 1/2 octave bands and 31 1/3 octave bands
	QVector<float> result(11, 0.0f);
	float * resultData = result.data();
	//start frequency of first octave range is always 15.625 Hz and doubles every (full) octave band
	float centerFrequency = 15.625f;
	const float rangeFactor = std::pow(2.0f, 0.5f);
	//loop through all octaves, averaging the values that belong into the octave
	for (int i = 0; i < result.size(); ++i)
	{
		//claculate low and high value of frequency range around centerFrequency
		const float rangeLow = centerFrequency / rangeFactor;
		const float rangeHigh = centerFrequency * rangeFactor;
		//calculate start index. clamp this to >= 1, as bin 0 is the DC component of the signal
		int startRange = std::round(rangeLow / frequencyRangePerBin);
		startRange = startRange >= 1 ? startRange : 1;
		//calculate end index. clamp this to < fftBinSize, as bin fftBinSize is the Nyquist frequency component of the signal
		int endRange = std::round(rangeHigh / frequencyRangePerBin);
		endRange = endRange < fftBinSize ? endRange : (fftBinSize - 1);
		//accumulate bins
		for (int j = startRange; j <= endRange; ++j)
		{
			resultData[i] += src[j];
		}
		//divide by number of bins
		resultData[i] /= endRange - startRange + 1;
		//step to next center frequency
		centerFrequency *= 2.0f;
	}
	return result;
}

void ProcessingWorker::normalizeValuesSQNR(float * dest, const float * src, const int size, const float sqnrValue)
{
	for (int i = 0; i < size; ++i)
	{
		dest[i] = (sqnrValue + src[i]) / sqnrValue;
	}
}
