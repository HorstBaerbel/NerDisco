#pragma once

class QObject;


class I_MIDIControl
{
public:
	/// @brief Retrieve this objects QObject pointer.
	/// @return QObject pointer to object.
	virtual QObject * toQObject() = 0;

public slots:
	/// @brief Change the value of a control from MIDI.
	/// @param value New value in the range [0,1].
	virtual void setValueFromMIDI(float value) = 0;

signals:
	/// @brief The value of the control has been changed through the GUI and the new value is sent.
	/// @param control This controls adress.
	/// @param value New value in the range [0,1].
	/// @brief Please define this signal in derived classes.
	void guiValueChanged(I_MIDIControl * control, float value);
};
