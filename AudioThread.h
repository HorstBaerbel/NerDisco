#pragma once

#include <QAudioProbe>
#include <QAudioRecorder>
#include <QThread>


class AudioThread : public QThread
{
    Q_OBJECT

public:
    AudioThread(QObject *parent = 0);
    ~AudioThread();

private:
    QAudioRecorder * m_audioRecorder;
    QAudioProbe * m_probe;
};
