#include "AudioThread.h"

#include <QAudioBuffer>
#include <QAudioDeviceInfo>


AudioThread::AudioThread(QObject *parent)
    : QThread(parent)
{
    m_audioRecorder = new QAudioRecorder(this);
    m_probe = new QAudioProbe;
    connect(m_probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(processBuffer(QAudioBuffer)));
    m_probe->setSource(m_audioRecorder);
}

QString AudioThread::currentInputDevice() const
{
    return m_currentInputDeviceName;
}

QStringList AudioThread::inputDeviceNames()
{
    QStringList deviceNames;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        deviceNames.append(deviceInfo.deviceName());
    }
    return deviceNames;
}

QString AudioThread::defaultInputDeviceName()
{
    return QAudioDeviceInfo::defaultInputDevice().deviceName();
}

QStringList AudioThread::ouputDeviceNames()
{
    QStringList deviceNames;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        deviceNames.append(deviceInfo.deviceName());
    }
    return deviceNames;
}

QString AudioThread::defaultOutputDeviceName()
{
    return QAudioDeviceInfo::defaultOutputDevice().deviceName();
}

AudioThread::~AudioThread()
{
}
