#include "DisplayThread.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>


DisplayThread::DisplayThread(QObject *parent)
	: QThread(parent)
	, m_waitTimeout(100)
	, m_quit(false)
	, portName("portName", "")
	, baudrate("baudrate", QSerialPort::Baud115200, QSerialPort::Baud1200, 500000)
	, sending("sendData", false)
	, displayWidth("displayWidth", 32, 8, 64)
	, displayHeight("displayHeight", 18, 4, 32)
	, displayInterval("displayInterval", 50, 20, 100)
	, flipHorizontal("flipHorizontal", false)
	, flipVertical("flipVertical", false)
	, scanlineDirection("scanlineDirection", ConstantLeftToRight)
{
	connect(portName.GetSharedParameter().get(), SIGNAL(valueChanged(const QString &)), this, SLOT(setPortName(const QString &)));
	connect(baudrate.GetSharedParameter().get(), SIGNAL(valueChanged(int)), this, SLOT(setBaudrate(int)));
	connect(sending.GetSharedParameter().get(), SIGNAL(valueChanged(bool)), this, SLOT(setSendData(bool)));
}

DisplayThread::~DisplayThread()
{
    //m_mutex.lock();
    m_quit = true;
    m_condition.wakeAll();
    //m_mutex.unlock();
    wait();
}

void DisplayThread::toXML(QDomElement & parent) const
{
	//try to find element in document
	QDomElement element = parent.firstChildElement("DisplayThread");
	if (element.isNull())
	{
		//add the new element
		element = parent.ownerDocument().createElement("DisplayThread");
		parent.appendChild(element);
	}
	portName.toXML(element);
	baudrate.toXML(element);
	displayWidth.toXML(element);
	displayHeight.toXML(element);
	displayInterval.toXML(element);
	flipHorizontal.toXML(element);
	flipVertical.toXML(element);
	scanlineDirection.toXML(element);
	sending.toXML(element);
}

DisplayThread & DisplayThread::fromXML(const QDomElement & parent)
{
	//try to find element in document
	QDomElement element = parent.firstChildElement("DisplayThread");
	if (element.isNull())
	{
		throw std::runtime_error("No audio device settings found!");
	}
	portName.fromXML(element);
	baudrate.fromXML(element);
	displayWidth.fromXML(element);
	displayHeight.fromXML(element);
	displayInterval.fromXML(element);
	flipHorizontal.fromXML(element);
	flipVertical.fromXML(element);
	scanlineDirection.fromXML(element);
	sending.fromXML(element);
	return *this;
}

QStringList DisplayThread::getAvailablePortNames() const
{
	QStringList result;
	foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		result.append(info.portName());
	}
	return result;
}

void DisplayThread::setSendData(bool sendData)
{
	QMutexLocker locker(&m_mutex);
	sending = sendData;
	if (!isRunning())
	{
		start();
		setPriority(QThread::HighPriority);
	}
	else
		m_condition.wakeOne();
}

void DisplayThread::setPortName(const QString &name)
{
	QMutexLocker locker(&m_mutex);
	portName = name;
	if (!isRunning())
	{
		start();
		setPriority(QThread::HighPriority);
	}
	else
		m_condition.wakeOne();
}

void DisplayThread::setBaudrate(int rate)
{
	QMutexLocker locker(&m_mutex);
	baudrate = rate;
	if (!isRunning())
	{
		start();
		setPriority(QThread::HighPriority);
	}
	else
		m_condition.wakeOne();
}

void DisplayThread::sendImage(const QImage & image, int waitTimeout)
{
    QMutexLocker locker(&m_mutex);
    m_waitTimeout = waitTimeout;
    m_displayImage = image;
    if (!isRunning())
	{
		start();
		setPriority(QThread::HighPriority);
	}
    else
        m_condition.wakeOne();
}

void DisplayThread::run()
{
	QString currentPortName = portName;
    bool currentPortNameChanged = true;
	int currentBaudrate = baudrate;
	bool currentBaudrateChanged = false;
	bool sendData = sending;
    QSerialPort serial;
    QByteArray data;

    while (!m_quit)
	{
        m_mutex.lock();
        if (currentPortName != portName) {
			currentPortName = portName;
            currentPortNameChanged = true;
        }
		if (currentBaudrate != baudrate) {
			currentBaudrate = baudrate;
			currentBaudrateChanged = true;
		}
		if (sendData != sending) {
			sendData = sending;
		}
		//get settings
		int waitTimeout = m_waitTimeout;
		const int width = displayWidth;
		const int height = displayHeight;
		const bool horizontal = flipHorizontal;
		const bool vertical = flipVertical;
		const ScanlineDirection direction = scanlineDirection;
		//convert image to format
		QImage dataImage;
		if (m_displayImage.format() != QImage::Format_ARGB32
			&& m_displayImage.format() != QImage::Format_ARGB32_Premultiplied
			&& m_displayImage.format() != QImage::Format_RGB32)
		{
			dataImage = m_displayImage.convertToFormat(QImage::Format_ARGB32);
		}
		else {
			dataImage = m_displayImage;//.rgbSwapped();
		}
		m_mutex.unlock();
		//check if we have data and display setup is ok
		if (!dataImage.isNull() && width > 0 && height > 0)
		{
			//convert image to proper size
			if (dataImage.width() != width || dataImage.height() != height)
			{
				dataImage = dataImage.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			}
			//flip image according to settings
			if (horizontal || vertical)
			{
				dataImage = dataImage.mirrored(horizontal, vertical);
			}
			//set up display parameters
			const unsigned int count = width * height;
			const unsigned char hi = (count >> 8) & 0xFF;
			const unsigned char lo = count & 0xFF;
			const unsigned char checksum = hi ^ lo ^ 0x55;
			//store display parameters in data
			data.clear();
			data.append(QString("Ada").toLatin1());
			data.append(hi);
			data.append(lo);
			data.append(checksum);
			//convert image for diplay. get initial scan line direction
			int stepDirection = ((direction == ConstantRightToLeft) || (direction == AlternatingStartRight)) ? 4 : -4;
			for (int y = 0; y < dataImage.height(); ++y)
			{
				const unsigned char * scanLine = dataImage.constScanLine(y);
				//loop through pixels left-right
				if (stepDirection > 0)
				{
					for (int x = 0; x < dataImage.width() * 4; x += 4)
					{
						data.append(scanLine[x + 1]);
						data.append(scanLine[x + 2]);
						data.append(scanLine[x]);
					}
				}
				else
				{
					for (int x = (dataImage.width() - 1) * 4; x >= 0; x -= 4)
					{
						data.append(scanLine[x + 1]);
						data.append(scanLine[x + 2]);
						data.append(scanLine[x]);
					}
				}
				//if we have alternating lines, flip direction after every line
				if ((direction == AlternatingStartLeft) || (direction == AlternatingStartRight))
				{
					stepDirection = -stepDirection;
				}
			}
		}
		//open new serial port device
		if (currentPortNameChanged)
		{
			//close old port down
			serial.close();
			sending = false;
			portName = currentPortName;
			currentPortNameChanged = false;
			//check if a proper port name was passed
			if (!currentPortName.isEmpty())
			{
				serial.setPortName(currentPortName);
				//try opening
				if (!serial.open(QIODevice::ReadWrite))
				{
					emit error(tr("Can't open %1, error code %2").arg(currentPortName).arg(serial.error()));
					emit portOpened(false);
				}
				else
				{
					//set up serial port
					serial.setBaudRate(currentBaudrate);
					serial.setDataBits(QSerialPort::Data8);
					serial.setParity(QSerialPort::NoParity);
					serial.setStopBits(QSerialPort::OneStop);
					serial.setFlowControl(QSerialPort::NoFlowControl);
					serial.setBreakEnabled(false);
					emit portOpened(true);
				}
			}
		}
		if (currentBaudrateChanged)
		{
			serial.setBaudRate(currentBaudrate);
			currentBaudrateChanged = false;
			baudrate = currentBaudrate;
		}
		if (sendData && serial.isOpen() && serial.isWritable() && data.size() > 0)
		{
			//read some data from the device so serial port is not overrun
			serial.readAll();
			//now write request
			serial.write(data);
			if (serial.waitForBytesWritten(waitTimeout)) {
				emit response("Sent");
			}
			else {
				emit timeout(tr("Wait write request timeout %1").arg(QTime::currentTime().toString()));
			}
		}
        m_mutex.lock();
        m_condition.wait(&m_mutex);
		if (m_quit)
		{
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
    }
}
