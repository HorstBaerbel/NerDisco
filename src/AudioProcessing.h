#pragma once

#include <QObject>
#include <QVector>
#include <QQueue>


//define to use KissFFT for FFT analysis and beat detection
//#define USE_KISSFFT

#ifdef USE_KISSFFT
	//Forward declarations for not having to include kissfft in header
	#ifdef __cplusplus
	extern "C" {
	#endif
		struct kiss_fftr_state;
		typedef struct kiss_fftr_state *kiss_fftr_cfg;
		struct kiss_fft_cpx;
	#ifdef __cplusplus
	}
	#endif
#endif

//worker class used in the interface
class ProcessingWorker : public QObject
{
	Q_OBJECT

public:
	ProcessingWorker(int sampleRate = 44100, QObject *parent = 0);
	~ProcessingWorker();

	void setSampleRate(int sampleRate = 44100);

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
	QVector<float> doFFT(const float * data, int channels);

	bool m_doFFT;
	bool m_doBeatDetection;
	bool m_doLevels;

	/// @brief Input data remaining from previous input() call.
	const QVector<float> m_previousInputData;
	/// @brief Minimum beat length for a beat to be detected. Default is 100ms.
	float m_beatLengthUs = 10 * 1000;
	/// @brief Sample rate of input data in Hz.
	int m_sampleRate = 44100;
	/// @brief FFT window size. Depends on the sample rate.
	int m_fftWindowSize = 1024;
	/// @brief FFT bin size. Depends on window/data size.
	int m_fftBinSize = m_fftWindowSize / 2 + 1;

#ifdef USE_KISSFFT
	/// @brief Configuration for KissFFT algorithm.
	kiss_fftr_cfg m_fftConfig = nullptr;
	/// @brief KissFFT real/imaginary results.
	kiss_fft_cpx * m_fftResult = nullptr;
#endif

	QQueue< QVector<int> > m_previousPeakPositions;
};
