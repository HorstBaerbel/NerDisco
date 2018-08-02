#pragma once

#include "Deck.h"
#include "DisplayThread.h"
//#include "AudioInterface.h"
#include "SignalJoiner.h"
#include "MIDIInterface.h"
#include "MIDIParameterMapping.h"
#include "DisplayImageConverter.h"
#include "Parameters.h"

#include <QMainWindow>
#include <QTimer>


namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	/// @brief Save the current settings to an XML document.
	/// @param parent The paren element to write the settings to.
	void toXML(QDomElement & parent) const;
	/// @brief Read current settings from XML document.
	/// @param parent The parent element to load the settings from.
	MainWindow & fromXML(const QDomElement & parent);

	void loadSettings(const QString & fileName);
	void saveSettings(const QString & fileName);

	ParameterInt previewInterval;
	ParameterInt frameBufferWidth;
	ParameterInt frameBufferHeight;
	ParameterInt displayInterval;
	ParameterInt displayWidth;
	ParameterInt displayHeight;
	ParameterInt displayGamma; //[150,350]
	ParameterInt displayBrightness; //[-50,50]
	ParameterInt displayContrast; //[-50,50]
	ParameterInt crossFadeValue; //[0,100]

protected slots:
    void updateDeckImages();
	void grabDeckImages();
	void updatePreview(const QImage & image);
	void updateDisplay(const QImage & image);

	void setDisplayWidth(int width);
	void setDisplayHeight(int height);
	void resizeDisplayLabels();

//	void updateAudioDevices();
//    void audioInputDeviceSelected();
//    void audioInputDeviceChanged(const QString & name);
//    void audioRecordTriggered(bool checked);
//    void audioStopTriggered();
//    void audioCaptureStateChanged(bool capturing);
//    void audioUpdateLevels(const QVector<float> & data, float timeus);
//	void audioUpdateFFT(const QVector<float> & spectrum, int channels, float timeus);

	void updateMidiDevices();
	void midiInputDeviceSelected();
	void midiInputDeviceChanged(const QString & name);
	void midiStartTriggered(bool checked);
	void midiStopTriggered();
	void midiCaptureStateChanged(bool capturing);
	void midiLearnMappingToggled();
	void midiStoreLearnedConnection();
	void midiLearnedConnectionStateChanged(bool valid);

	void updateDisplaySerialPortMenu();
	void updateDisplaySettingsMenu();
	void displaySerialPortSelected();
	void displayBaudrateSelected();
	void displayScanlineDirectionSelected();
	void displaySerialPortChanged(const QString & name);
	void displayBaudrateChanged(int rate);
	void displayScanlineDirectionChanged(ScanlineDirection direction);
	void displayStartSending(bool checked);
	void displayStopSending();
	void displayPortStatusChanged(bool opened);
	void displaySendStatusChanged(bool sending);
	void displayFlipChanged(bool horizontal, bool vertical);

	void updateScreenMenu();

	void updateEffectMenu();
	void updateDeckMenu();
    void loadDeckA(bool checked = false);
    void saveDeckA(bool checked = false);
    void saveAsDeckA(bool checked = false);
    void loadDeckB(bool checked = false);
    void saveDeckB(bool checked = false);
    void saveAsDeckB(bool checked = false);

    void showResponse(const QString &s);
    void processError(const QString &s);
    void processTimeout(const QString &s);

public slots:
	void exitApplication();

private:
    Ui::MainWindow *ui;

    QTimer m_displayTimer;
	QString m_settingsFileName;

	DisplayImageConverter m_displayImageConverter;
    DisplayThread m_displayThread;
//    AudioInterface m_audioInterface;
	SignalJoiner m_signalJoiner;
	MIDIInterface::SPtr m_midiInterface;
};
