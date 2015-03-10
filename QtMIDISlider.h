#pragma once

#include <QSlider>
#include "I_MIDIControl.h"


class QtMIDISlider: public QSlider, public I_MIDIControl
{
	Q_OBJECT

public:
	QtMIDISlider(QWidget * parent = NULL);
	QtMIDISlider(Qt::Orientation orientation, QWidget * parent = NULL);

	/// @copydoc I_MIDIControl::toQObject
	QObject * toQObject();

public slots:
	/// @copydoc I_MIDIControl::setValueFromMIDI
	virtual void setValueFromMIDI(float value);

protected slots:
	/// @brief Slot that gets called when the GUI value changes.
	/// @param New slider value.
	virtual void setValueFromGui(int value);

signals:
	/// @copydoc I_MIDIControl::guiValueChanged
	void guiValueChanged(I_MIDIControl * control, float value);
};