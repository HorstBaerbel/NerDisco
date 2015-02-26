#pragma once

#include <QString>
#include <QSettings>


class Settings
{
public:
    enum ScanlineDirection {ConstantLeftToRight, ConstantRightToLeft,
                            AlternatingStartLeft, AlternatingStartRight};

    static Settings & getInstance();

    QString portName() const;
    void setPortName(const QString & name);

    QString audioSinkName() const;
    void setAudioSinkName(const QString & name);

    unsigned int displayWidth() const;
    void setDisplayWidth(unsigned int width);
    unsigned int displayHeight() const;
    void setDisplayHeight(unsigned int height);

    unsigned int frameBufferWidth() const;
    void setFrameBufferWidth(unsigned int width);
    unsigned int frameBufferHeight() const;
    void setFrameBufferHeight(unsigned int height);

    unsigned int displayInterval() const;
    void setDisplayInterval(unsigned int interval);

    bool displayFlipHorizontal() const;
    void setDisplayFlipHorizontal(bool flip);

    bool displayFlipVertical() const;
    void setDisplayFlipVertical(bool flip);

    float displayGamma() const;
    void setDisplayGamma(float gamma);

    float displayBrightness() const;
    void setDisplayBrightness(float brightness);

    float displayContrast() const;
    void setDisplayContrast(float contrast);

    ScanlineDirection scanlineDirection() const;
    void setScanlineDirection(ScanlineDirection direction);

    QString audioInputDeviceName() const;
    void setAudioInputDeviceName(const QString & name);

	QString midiInputDeviceName() const;
	void setMidiInputDeviceName(const QString & name);

    void save();

    ~Settings();

private:
    Settings();
    Settings(Settings & other);

    QSettings * m_settings;
    QString m_portName;
    unsigned int m_displayWidth;
    unsigned int m_displayHeight;
    unsigned int m_frameBufferWidth;
    unsigned int m_frameBufferHeight;
    unsigned int m_displayInterval;
    bool m_displayFlipHorizontal;
    bool m_displayFlipVertical;
    float m_displayGamma;
    float m_displayBrightness;
    float m_displayContrast;
    ScanlineDirection m_scanlineDirection;
    QString m_audioSinkName;
    QString m_audioInputDeviceName;
	QString m_midiInputDeviceName;

    static Settings s_settings;
};
