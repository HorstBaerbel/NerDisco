#include "QtMIDIButton.h"


QtMIDIButton::QtMIDIButton(QWidget * parent)
	: QPushButton(parent)
{
	connect(this, SIGNAL(pressed()), this, SLOT(setPressedFromGui()));
	connect(this, SIGNAL(released()), this, SLOT(setReleasedFromGui()));
}

QObject * QtMIDIButton::toQObject()
{
	return this;
}

void QtMIDIButton::setPressedFromGui()
{
	emit guiValueChanged(this, 1.0f);
}

void QtMIDIButton::setReleasedFromGui()
{
	emit guiValueChanged(this, 0.0f);
}

void QtMIDIButton::setValueFromMIDI(float value)
{
	setDown(value >= 0.5f);
	//now emit values, as the buttoin itself won't emit signals
	//when setDown is called programmatically. why. ever.
	if (value >= 0.5f)
	{
		emit pressed();
	}
	else
	{
		emit released();
	}
}
