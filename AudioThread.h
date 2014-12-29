#pragma once

#include <QAudioProbe>
#include <QAudioRecorder>
#include <QThread>


class AudioThread : public QThread
{
    Q_OBJECT

public:
    AudioThread(QObject *parent = 0);

    static QStringList inputDeviceNames();
    static QString defaultInputDeviceName();

    static QStringList ouputDeviceNames();
    static QString defaultOutputDeviceName();

    QString currentInputDevice() const;

    ~AudioThread();

private:
    QAudioRecorder * m_audioRecorder;
    QAudioProbe * m_probe;
    QString m_currentInputDeviceName;
};
