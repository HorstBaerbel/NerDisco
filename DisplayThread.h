#pragma once

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>


class DisplayThread : public QThread
{
    Q_OBJECT

public:
    DisplayThread(QObject *parent = 0);
    ~DisplayThread();

	void setSendData(bool sendData);
    void setPortName(const QString &name);
	void setBaudrate(int baudrate = 115200);
    void sendData(const QImage &displayImage, int m_waitTimeout = 100);
    void run();

signals:
	void portOpened(bool portOpen);
	void sendingData(bool sendsData);
    void response(const QString &s);
    void error(const QString &s);
    void timeout(const QString &s);

private:
    QString m_portName;
	int m_baudrate;
    QImage m_displayImage;
    int m_waitTimeout;
	bool m_sendData;
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_quit;
};
