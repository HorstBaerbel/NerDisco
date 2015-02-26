#pragma once

#include "Deck.h"
#include "DisplayThread.h"
#include "AudioInterface.h"
#include "SignalJoiner.h"
#include "MIDIInterface.h"
#include "MIDIControlMapper.h"

#include <QMainWindow>
#include <QTimer>


namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QImage currentImage() const;
    QImage realImage() const;
    ~MainWindow();

protected slots:
    void updateDeckImages();
	void grabDeckImages();

    void updateMenu();
    void updateSettingsFromUi();
    void updateAudioDevices();
	void updateMidiDevices();

    void audioInputDeviceSelected();
    void audioInputDeviceChanged(const QString & name);
    void audioRecordTriggered(bool checked);
    void audioStopTriggered();
    void audioCaptureStateChanged(bool capturing);
    void audioUpdateLevels(const QVector<float> & data, float timeus);

	void midiInputDeviceSelected();
	void midiInputDeviceChanged(const QString & name);
	void midiStartTriggered(bool checked);
	void midiStopTriggered();
	void midiCaptureStateChanged(bool capturing);
	void midiLearnMappingToggled(bool checked);
	void midiStoreLearnedConnection();
	void midiLearnedConnectionStateChanged(bool valid);

    void loadDeckA(bool checked = false);
    void saveDeckA(bool checked = false);
    void saveAsDeckA(bool checked = false);
    void loadDeckB(bool checked = false);
    void saveDeckB(bool checked = false);
    void saveAsDeckB(bool checked = false);

    void showResponse(const QString &s);
    void processError(const QString &s);
    void processTimeout(const QString &s);

	void crossFaderValueChanged(int value);

public slots:
    void showSettings();
    void exitApplication();

	/// @brief Change the value of a control in the deck.
	/// @param controlName Name of control to change.
	/// @param value New value in the range [0,1].
	/// @note The name of the control is tha same as in the signal void valueChanged(const QString & controlName, float value).
	void setValue(const QString & controlName, float value);

signals:
	/// @brief The value of a control in the window has changed and the new value is sent.
	/// @param controlName Name of control that has changed.
	/// @param value New value in the range [0,1].
	/// @note The name of the control can be used in void setValue(const QString & which, float value) to set a value.
	void valueChanged(const QString & controlName, float value);

private:
    Ui::MainWindow *ui;
    QTimer m_displayTimer;
    QImage m_currentImage;
    QImage m_realImage;
    DisplayThread m_displayThread;
    AudioInterface m_audioInterface;
	SignalJoiner m_signalJoiner;
	MIDIInterface m_midiInterface;
	MIDIControlMapper m_midiMapper;
};
