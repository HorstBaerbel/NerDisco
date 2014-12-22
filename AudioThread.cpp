#include "AudioThread.h"

#include <QAudioBuffer>


AudioThread::AudioThread(QObject *parent)
    : QThread(parent)
{
    m_audioRecorder = new QAudioRecorder(this);
    m_probe = new QAudioProbe;
    connect(m_probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(processBuffer(QAudioBuffer)));
    m_probe->setSource(m_audioRecorder);
}

AudioThread::~AudioThread()
{
}
