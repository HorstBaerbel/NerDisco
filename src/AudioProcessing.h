#pragma once

#include <QObject>
#include <QVector>
#include <QQueue>

//Forward declarations for not having to include kissfft in header here
#ifdef __cplusplus
extern "C" {
#endif
	struct kiss_fftr_state;
	typedef struct kiss_fftr_state *kiss_fftr_cfg;
	struct kiss_fft_cpx;
#ifdef __cplusplus
}
#endif

//worker class used in the interface
class ProcessingWorker : public QObject
{
	Q_OBJECT

public:
	ProcessingWorker(int sampleRate = 44100, int bitDepth = 8, QObject *parent = 0);
	~ProcessingWorker();

	void setSampleRate(int sampleRate = 44100);
	void setBitDepth(int bitDepth = 8);

	void enableLevelsData(bool enable = true);
	void enableBeatData(bool enable = false);
	void enableFFTData(bool enable = false);

signals:
	/// @brief Delivers audio levels for each channel.
	void levelData(const QVector<float> & levels, float timeus);
	/// @brief Delivers the FFT of the current audio data.
	void fftData(const QVector<float> & spectrum, int channels, float timeus);
	/// @brief Delivers beat information for the beat detection.
	void beatData(float bpm, bool isBeat);

	void output(const QVector<float> & data, int channels, float timeus);

public slots:
	void input(const QVector<float> & data, int channels, float timeus);

private:
	/// @brief Update the window coefficients.
	void UpdateWindowCoefficients();
	/// @brief Update the kiss configuration.
	void UpdateKissConfig();
	/// @brief Get maximum levels for each channel.
	QVector<float> getMaximumLevels(const QVector<float> & data, int channels);
	/// @brief Apply the pre-calculated window function to a bit of audio data.
	void applyWindowFunction(float * dest, const float * src, const float * windowData, const int nrOfSamples, int channels);
	/// @brief Normalize the complex fft result using the FFT size and sum of the window function coefficients.
	void normalizeFFTResult(kiss_fft_cpx * complex, const int fftBinSize, const int fftWindowSize, const float windowFunctionCoefficientSum);
	/// @brief Calculate the signal magnitude from the complex FFT results.
	void calculateSquareMagnitude(float * dest, const kiss_fft_cpx * complexResult, const int fftBinSize);
	/// @brief Calculate magnitude in dB from square magnitude.
	void calculateMagnitudedB(float * dest, const float * src, const int fftBinSize);
	QVector<float> averageBands(const float * src, const int fftBinSize, const int factor);
	/// @brief Calculate average spectrum data into octave bands.
	QVector<float> calculateOctaveBands(const float * src, const int fftBinSize, const int windowSize, const int sampleRate);
	/// @brief Normalize spectrum values using the SQNR value calculated from the bit depth.
	void normalizeValuesSQNR(float * dest, const float * src, const int size, const float sqnrValue);

	bool m_doFFT = true;
	bool m_doBeatDetection = false;
	bool m_doLevels = true;

	/// @brief Input data remaining from previous input() call.
	const QVector<float> m_previousInputData;
	/// @brief Minimum beat length for a beat to be detected. Default is 100ms.
	float m_beatLengthUs = 10 * 1000;
	/// @brief Sample rate of input data in Hz.
	int m_sampleRate = 44100;
	/// @brief Bit depth of audio signal.
	int m_bitDepth = 8;
	/// @brief The SQNR value in dB for the bitdepth set.
	float m_Sqnr = 48.16f;
	/// @brief FFT window size. Depends on the sample rate.
	int m_fftWindowSize = 4096;
	/// @brief FFT bin size. Depends on window/data size.
	int m_fftBinSize = m_fftWindowSize / 2 + 1;
	/// @brief Overlap for FFT in samples.
	int m_fftWindowOverlap = m_fftWindowSize / 8;
	/// @brief Configuration for KissFFT algorithm.
	kiss_fftr_cfg m_fftConfig = nullptr;
	/// @brief KissFFT real/imaginary results.
	kiss_fft_cpx * m_fftResult = nullptr;
	/// @brief Intermediate buffer for FFT preparation.
	float * m_intermediateBuffer = nullptr;
	/// @brief Window function coefficients for fft.
	float * m_windowFunctionCoefficients = nullptr;
	/// @brief Sum of all window coefficients used for normalization.
	float m_windowFunctionCoefficientSum = 0.0f;
	/// @brief Keeps data for current frame. Keeps unprocessed samples remaining from last frame.
	QVector<float> m_data;
	/// @brief Final spectrum results.
	QVector<float> m_spectrum;
	/// @brief Flag is true when the KissFFT configuration changed and needs to be updated.
	bool m_kissConfigChanged = true;

	QQueue< QVector<int> > m_previousPeakPositions;
};
