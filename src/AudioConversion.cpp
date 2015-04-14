#include "AudioConversion.h"


ConversionWorker::ConversionWorker(QObject *parent)
	: QObject(parent)
	, m_convertToMono(false)
{
	qRegisterMetaType< QVector<float> >("QVector<float>");
}

void ConversionWorker::input(const QByteArray & buffer, const QAudioFormat & format)
{
	const float duration = getDuration(buffer, format);
	QVector<float> floatData = converToFloat(buffer, format);
	int channels = format.channelCount();
	if (m_convertToMono && channels > 1)
	{
		floatData = convertToMono(floatData, channels);
		channels = 1;
	}
	emit output(floatData, channels, duration);
}

float ConversionWorker::getDuration(const QByteArray & buffer, const QAudioFormat & format)
{
	if (!format.isValid())
		return 0.0f;
	if (format.codec() != "audio/pcm")
		return 0.0f;
	if (buffer.size() <= 0)
		return 0.0f;
	return (float)(format.durationForBytes(buffer.size()));
}

float ConversionWorker::getPeakValue(const QAudioFormat& format)
{
	// Note: Only the most common sample formats are supported
	if (!format.isValid())
		return 0.0f;
	if (format.codec() != "audio/pcm")
		return 0.0f;

	switch (format.sampleType()) {
	case QAudioFormat::Unknown:
		break;
	case QAudioFormat::Float:
		if (format.sampleSize() != 32) // other sample formats are not supported
			return qreal(0);
		return 1.00003f;
	case QAudioFormat::SignedInt:
		if (format.sampleSize() == 32)
			return (float)INT_MAX;
		if (format.sampleSize() == 16)
			return (float)SHRT_MAX;
		if (format.sampleSize() == 8)
			return (float)CHAR_MAX;
		break;
	case QAudioFormat::UnSignedInt:
		if (format.sampleSize() == 32)
			return (float)UINT_MAX;
		if (format.sampleSize() == 16)
			return (float)USHRT_MAX;
		if (format.sampleSize() == 8)
			return (float)UCHAR_MAX;
		break;
	}
	return 0.0f;
}

template <class T>
QVector<float> converBuffer(const T *buffer, int frames, int channels)
{
	QVector<float> values(frames*channels);
	for (int i = 0; i < frames; ++i) {
		for (int j = 0; j < channels; ++j) {
			values[i * channels + j] = (float)buffer[i * channels + j];
		}
	}
	return values;
}

QVector<float> ConversionWorker::converToFloat(const QByteArray & buffer, const QAudioFormat & format)
{
	QVector<float> values;

	if (!format.isValid() || format.byteOrder() != QAudioFormat::LittleEndian)
		return values;
	if (format.codec() != "audio/pcm")
		return values;
	if (buffer.size() <= 0)
		return values;

	const int frames = format.framesForBytes(buffer.size());
	const int channelCount = format.channelCount();
	values.fill(0.0f, channelCount);
	float peak_value = getPeakValue(format);
	if (qFuzzyCompare(peak_value, 0.0f))
		return values;

	switch (format.sampleType()) {
	case QAudioFormat::Unknown:
	case QAudioFormat::UnSignedInt:
		//convert values to float, but keep range
		if (format.sampleSize() == 32)
		{
			values = converBuffer((quint32*)buffer.constData(), frames, channelCount);
		}
		else if (format.sampleSize() == 16)
		{
			values = converBuffer((quint16*)buffer.constData(), frames, channelCount);
		}
		else if (format.sampleSize() == 8)
		{
			values = converBuffer((quint8*)buffer.constData(), frames, channelCount);
		}
		//normalize values to [-1,1]
		for (int i = 0; i < values.size(); ++i)
		{
			values[i] = qAbs(values.at(i) - 0.5f * peak_value) / (0.5f * peak_value);
		}
		break;
	case QAudioFormat::Float:
		//convert values to float, but keep range
		if (format.sampleSize() == 32)
		{
			values = converBuffer((float*)buffer.constData(), frames, channelCount);
		}
		//normalize values to [-1,1]
		for (int i = 0; i < values.size(); ++i)
		{
			values[i] /= peak_value;
		}
		break;
	case QAudioFormat::SignedInt:
		//convert values to float, but keep range
		if (format.sampleSize() == 32)
		{
			values = converBuffer((qint32*)buffer.constData(), frames, channelCount);
		}
		else if (format.sampleSize() == 16)
		{
			values = converBuffer((qint16*)buffer.constData(), frames, channelCount);
		}
		if (format.sampleSize() == 8)
		{
			values = converBuffer((qint8*)buffer.constData(), frames, channelCount);
		}
		//normalize values to [-1,1]
		for (int i = 0; i < values.size(); ++i)
		{
			values[i] /= peak_value;
		}
		break;
	}
	return values;
}

QVector<float> ConversionWorker::convertToMono(const QVector<float> & data, int channels)
{
	if (channels > 1)
	{
		const int frames = data.size() / channels;
		QVector<float> mono(frames);
		for (int i = 0; i < frames; ++i) {
			float value = 0.0f;
			for (int j = 0; j < channels; ++j) {
				value += data[i * channels + j];
			}
			mono[i] = value / channels;
		}
		return mono;
	}
	else
	{
		return data;
	}
}

ConversionWorker::~ConversionWorker()
{
}
