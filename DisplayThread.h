#pragma once

#include "Parameters.h"
#include "ParameterScanlineDirection.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QDomDocument>
#include <QStringList>


class DisplayThread : public QThread
{
    Q_OBJECT

public:
    DisplayThread(QObject *parent = 0);
    ~DisplayThread();

	/// @brief Save the current settings to an XML document.
	/// @param parent The paren element to write the settings to.
	void toXML(QDomElement & parent) const;
	/// @brief Read current settings from XML document.
	/// @param parent The parent element to load the settings from.
	DisplayThread & fromXML(const QDomElement & parent);

	QStringList getAvailablePortNames() const;

	ParameterQString portName;
	ParameterInt baudrate;
	ParameterInt displayWidth;
	ParameterInt displayHeight;
	ParameterInt displayInterval;
	ParameterBool flipHorizontal;
	ParameterBool flipVertical;
	ParameterScanlineDirection scanlineDirection;
	ParameterBool sending;

    void sendImage(const QImage &displayImage, int m_waitTimeout = 100);

signals:
	void portOpened(bool portOpen);
    void response(const QString &s);
    void error(const QString &s);
    void timeout(const QString &s);

protected:
	void run();

private slots:
	void setSendData(bool sendData);
	void setPortName(const QString &name);
	void setBaudrate(int baudrate = 115200);

private:
    QImage m_displayImage;
    int m_waitTimeout;
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_quit;
};
