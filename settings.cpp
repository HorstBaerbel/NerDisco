#include "Settings.h"


Settings Settings::s_settings;

Settings::Settings()
    : m_settings(new QSettings("settings.ini", QSettings::IniFormat))
{
    m_portName = m_settings->value("SerialPortName").toString();
    m_displayWidth = m_settings->value("DisplayWidth", 1).toUInt();
    m_displayHeight = m_settings->value("DisplayHeight", 1).toUInt();
    m_frameBufferWidth = m_settings->value("FrameBufferWidth", 1).toUInt();
    m_frameBufferHeight = m_settings->value("FrameBufferHeight", 1).toUInt();
    m_displayInterval = m_settings->value("DisplayInterval", 50).toUInt();
    m_displayFlipHorizontal = m_settings->value("DisplayFlipHorizontal", false).toBool();
    m_displayFlipVertical = m_settings->value("DisplayFlipVertical", false).toBool();
    m_displayFlipVertical = m_settings->value("DisplayFlipVertical", false).toBool();
    m_scanlineDirection = (ScanlineDirection)m_settings->value("ScanlineDirection", (unsigned int)ConstantLeftToRight).toUInt();
    m_displayGamma = m_settings->value("DisplayGamma", 2.2f).toFloat();
    m_displayBrightness = m_settings->value("DisplayBrightness", 0.0f).toFloat();
    m_displayContrast = m_settings->value("DisplayContrast", 1.0f).toFloat();
    m_audioSinkName = m_settings->value("AudioSinkName").toString();
}

Settings::~Settings()
{
    delete m_settings;
}

void Settings::save()
{
    m_settings->setValue("SerialPortName", m_portName);
    m_settings->setValue("DisplayWidth", m_displayWidth);
    m_settings->setValue("DisplayHeight", m_displayHeight);
    m_settings->setValue("FrameBufferWidth", m_frameBufferWidth);
    m_settings->setValue("FrameBufferHeight", m_frameBufferHeight);
    m_settings->setValue("DisplayInterval", m_displayInterval);
    m_settings->setValue("DisplayFlipHorizontal", m_displayFlipHorizontal);
    m_settings->setValue("DisplayFlipVertical", m_displayFlipVertical);
    m_settings->setValue("ScanlineDirection", (unsigned int)m_scanlineDirection);
    m_settings->setValue("DisplayGamma", m_displayGamma);
    m_settings->setValue("DisplayBrightness", m_displayBrightness);
    m_settings->setValue("DisplayContrast", m_displayContrast);
    m_settings->setValue("AudioSinkName", m_audioSinkName);
    m_settings->sync();
}

Settings & Settings::getInstance()
{
    return s_settings;
}

QString Settings::portName() const
{
    return m_portName;
}

void Settings::setPortName(const QString & name)
{
    m_portName = name;
}

QString Settings::audioSinkName() const
{
    return m_audioSinkName;
}

void Settings::setAudioSinkName(const QString & name)
{
    m_audioSinkName = name;
}

unsigned int Settings::displayWidth() const
{
    return m_displayWidth;
}

void Settings::setDisplayWidth(unsigned int width)
{
    m_displayWidth = width;
}

unsigned int Settings::displayHeight() const
{
    return m_displayHeight;
}

void Settings::setDisplayHeight(unsigned int height)
{
    m_displayHeight = height;
}

unsigned int Settings::frameBufferWidth() const
{
    return m_frameBufferWidth;
}

void Settings::setFrameBufferWidth(unsigned int width)
{
    m_frameBufferWidth = width;
}

unsigned int Settings::frameBufferHeight() const
{
    return m_frameBufferHeight;
}

void Settings::setFrameBufferHeight(unsigned int height)
{
    m_frameBufferHeight = height;
}

unsigned int Settings::displayInterval() const
{
    return m_displayInterval;
}

void Settings::setDisplayInterval(unsigned int interval)
{
    m_displayInterval = interval;
}

bool Settings::displayFlipHorizontal() const
{
    return m_displayFlipHorizontal;
}

void Settings::setDisplayFlipHorizontal(bool flip)
{
    m_displayFlipHorizontal = flip;
}

bool Settings::displayFlipVertical() const
{
    return m_displayFlipVertical;
}

void Settings::setDisplayFlipVertical(bool flip)
{
    m_displayFlipVertical = flip;
}

Settings::ScanlineDirection Settings::scanlineDirection() const
{
    return m_scanlineDirection;
}

void Settings::setScanlineDirection(ScanlineDirection direction)
{
    m_scanlineDirection = direction;
}

float Settings::displayGamma() const
{
    return m_displayGamma;
}

void Settings::setDisplayGamma(float gamma)
{
    m_displayGamma = gamma;
}

float Settings::displayBrightness() const
{
    return m_displayBrightness;
}

void Settings::setDisplayBrightness(float brightness)
{
    m_displayBrightness = brightness;
}

float Settings::displayContrast() const
{
    return m_displayContrast;
}

void Settings::setDisplayContrast(float contrast)
{
    m_displayContrast = contrast;
}
