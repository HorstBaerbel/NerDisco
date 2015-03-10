#include "QtMIDIDial.h"


QtMIDIDial::QtMIDIDial(QWidget * parent)
	: QDial(parent)
{
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(setValueFromGui(int)));
}

QObject * QtMIDIDial::toQObject()
{
	return this;
}

void QtMIDIDial::setValueFromGui(int value)
{
	emit guiValueChanged(this, (float)(value - minimum()) / (float)(maximum() - minimum()));
}

void QtMIDIDial::setValueFromMIDI(float value)
{
	setValue(value * (maximum() - minimum()) + minimum());
}
