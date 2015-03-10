#pragma once

#include <QDial>
#include "I_MIDIControl.h"


class QtMIDIDial : public QDial, public I_MIDIControl
{
	Q_OBJECT

public:
	QtMIDIDial(QWidget * parent = NULL);

	/// @copydoc I_MIDIControl::toQObject
	QObject * toQObject();

	public slots:
	/// @copydoc I_MIDIControl::setValueFromMIDI
	virtual void setValueFromMIDI(float value);

protected slots:
	/// @brief Slot that gets called when the GUI value changes.
	/// @param New dial value.
	virtual void setValueFromGui(int value);

signals:
	/// @copydoc I_MIDIControl::guiValueChanged
	void guiValueChanged(I_MIDIControl * control, float value);
};