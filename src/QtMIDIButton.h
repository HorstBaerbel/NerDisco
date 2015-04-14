#pragma once

#include <QPushButton>
#include "I_MIDIControl.h"


class QtMIDIButton : public QPushButton, public I_MIDIControl
{
	Q_OBJECT

public:
	QtMIDIButton(QWidget * parent = NULL);

	/// @copydoc I_MIDIControl::toQObject
	QObject * toQObject();

public slots:
	/// @copydoc I_MIDIControl::setValueFromMIDI
	virtual void setValueFromMIDI(float value);

protected slots:
	/// @brief Slot that gets called when the GUI value changes.
	virtual void setPressedFromGui();
	/// @brief Slot that gets called when the GUI value changes.
	virtual void setReleasedFromGui();

signals:
	/// @copydoc I_MIDIControl::guiValueChanged
	void guiValueChanged(I_MIDIControl * control, float value);
};