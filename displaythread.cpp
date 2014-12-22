#include "displaythread.h"
#include "settings.h"

#include <QtSerialPort/QSerialPort>
#include <QTime>

QT_USE_NAMESPACE

DisplayThread::DisplayThread(QObject *parent)
    : QThread(parent), m_waitTimeout(0), m_quit(false)
{
}

DisplayThread::~DisplayThread()
{
    m_mutex.lock();
    m_quit = true;
    m_condition.wakeOne();
    m_mutex.unlock();
    wait();
}

void DisplayThread::setPortName(const QString &name)
{
    m_portName = name;
}

void DisplayThread::sendData(const QImage & image, int waitTimeout)
{
    QMutexLocker locker(&m_mutex);
    this->m_waitTimeout = waitTimeout;
    this->m_displayImage = image;
    if (!isRunning())
        start();
    else
        m_condition.wakeOne();
}

void DisplayThread::run()
{
    QString currentPortName;
    bool currentPortNameChanged = false;
    QSerialPort serial;
    QByteArray data;

    while (!m_quit) {
        m_mutex.lock();
        if (currentPortName != m_portName) {
            currentPortName = m_portName;
            currentPortNameChanged = true;
        }
        //set up display parameters
        Settings & settings = Settings::getInstance();
        const unsigned int count = settings.displayWidth() * settings.displayHeight();
        const unsigned char hi = (count >> 8) & 0xFF;
        const unsigned char lo = count & 0xFF;
        const unsigned char checksum = hi ^ lo ^ 0x55;
        //store display parameters in data
        data.clear();
        data.append(QString("Ada").toLatin1());
        data.append(hi);
        data.append(lo);
        data.append(checksum);
        //convert image to format
        QImage dataImage;
        if (dataImage.format() != QImage::Format_ARGB32
                && dataImage.format() != QImage::Format_ARGB32_Premultiplied
                && dataImage.format() != QImage::Format_RGB32)
        {
            dataImage = m_displayImage.convertToFormat(QImage::Format_ARGB32);
        }
        else {
            dataImage = m_displayImage;//.rgbSwapped();
        }
        m_mutex.unlock();
        //convert image to proper size
        if (dataImage.width() != settings.displayWidth() || dataImage.height() != settings.displayHeight())
        {
            dataImage = dataImage.scaled(settings.displayWidth(), settings.displayHeight(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        //flip image according to settings
        if (settings.displayFlipHorizontal() || settings.displayFlipVertical())
        {
            dataImage = dataImage.mirrored(settings.displayFlipHorizontal(), settings.displayFlipVertical());
        }
        //convert image for diplay. get initial scan line direction
        int direction = settings.scanlineDirection() == Settings::ConstantRightToLeft || settings.scanlineDirection() == Settings::AlternatingStartRight ? 4 : -4;
        for (int y = 0; y < dataImage.height(); ++y)
        {
            const unsigned char * scanLine = dataImage.constScanLine(y);
            //loop through pixels left-right
            if (direction > 0)
            {
                for (int x = 0; x < dataImage.width()*4; x+=4)
                {
                    data.append(scanLine[x+2]);
                    data.append(scanLine[x+1]);
                    data.append(scanLine[x]);
                }
            }
            else
            {
                for (int x = (dataImage.width()-1)*4; x >= 0; x-=4)
                {
                    data.append(scanLine[x+2]);
                    data.append(scanLine[x+1]);
                    data.append(scanLine[x]);
                }
            }
            //if we have alternating lines, flip direction after every line
            if (settings.scanlineDirection() == Settings::AlternatingStartLeft || settings.scanlineDirection() == Settings::AlternatingStartRight)
            {
                direction = -direction;
            }
        }
        //open serial port
        if (currentPortNameChanged) {
            serial.close();
            serial.setPortName(currentPortName);

            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(tr("Can't open %1, error code %2").arg(m_portName).arg(serial.error()));
                return;
            }
            //set up serial port
            serial.setBaudRate(QSerialPort::Baud115200);
            serial.setDataBits(QSerialPort::Data8);
            serial.setParity(QSerialPort::NoParity);
            serial.setStopBits(QSerialPort::OneStop);
            //serial.setFlowControl(QSerialPort::HardwareControl);
            serial.setBreakEnabled(false);
            currentPortNameChanged = false;
        }
        if (serial.isOpen() && serial.isWritable())
        {
            // write request
            serial.write(data);
            if (serial.waitForBytesWritten(m_waitTimeout)) {
                emit this->response("Sent");
            } else {
                emit timeout(tr("Wait write request timeout %1").arg(QTime::currentTime().toString()));
            }
        }
        m_mutex.lock();
        m_condition.wait(&m_mutex);
        m_mutex.unlock();
    }
}
