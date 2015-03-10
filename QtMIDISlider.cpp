#include "QtMIDISlider.h"


QtMIDISlider::QtMIDISlider(QWidget * parent)
	: QSlider(parent)
{
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(setValueFromGui(int)));
}

QtMIDISlider::QtMIDISlider(Qt::Orientation orientation, QWidget * parent)
	: QSlider(orientation, parent)
{
	connect(this, SIGNAL(valueChanged(in)), this, SLOT(setValueFromGui(int)));
}

QObject * QtMIDISlider::toQObject()
{
	return this;
}

void QtMIDISlider::setValueFromGui(int value)
{
	emit guiValueChanged(this, (float)(value - minimum()) / (float)(maximum() - minimum()));
}

void QtMIDISlider::setValueFromMIDI(float value)
{
	setValue(value * (maximum() - minimum()) + minimum());
}
