#pragma once

#include <QObject>
#include <QVector>
#include <QAudioFormat>


//worker class used in the interface
class ConversionWorker : public QObject
{
	Q_OBJECT

public:
	ConversionWorker(QObject *parent = 0);
	~ConversionWorker();

	void convertToMono(bool mono = true);

signals:
	void output(const QVector<float> & data, int channels, float timeus);

public slots:
	void input(const QByteArray & buffer, const QAudioFormat & format);

private:
	//Calculate audio duration in us based on data size and audio format.
	float getDuration(const QByteArray & buffer, const QAudioFormat & format);
	//Returns the maximum possible sample value for a given audio format.
	float getPeakValue(const QAudioFormat & format);
	//Convert arbitrary buffer to float in range [-1,1].
	QVector<float> converToFloat(const QByteArray & buffer, const QAudioFormat & format);
	//Convert buffer for arbitrary number of channels to mono format.
	QVector<float> convertToMono(const QVector<float> & data, int channels);

	bool m_convertToMono;
};
