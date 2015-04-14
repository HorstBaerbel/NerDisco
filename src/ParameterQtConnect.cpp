#include "ParameterQtConnect.h"


void connect(NodeRanged::SPtr parameter, QAbstractSlider * slider)
{
	if (!parameter || !slider)
	{
		throw std::runtime_error("connectParameter() - Invalid parameter(s) passed!");
	}
	slider->setRange(parameter->minRange(), parameter->maxRange());
	slider->setValue(parameter->value());
	QObject::connect(slider, SIGNAL(valueChanged(int)), parameter.get(), SLOT(setValue(int)));
	QObject::connect(parameter.get(), SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
}

void connect(NodeRanged::SPtr parameter, QSpinBox * spinbox)
{
	if (!parameter || !spinbox)
	{
		throw std::runtime_error("connectParameter() - Invalid parameter(s) passed!");
	}
	spinbox->setRange(parameter->minRange(), parameter->maxRange());
	spinbox->setValue(parameter->value());
	QObject::connect(spinbox, SIGNAL(valueChanged(int)), parameter.get(), SLOT(setValue(int)));
	QObject::connect(parameter.get(), SIGNAL(valueChanged(int)), spinbox, SLOT(setValue(int)));
}

void connect(NodeRanged::SPtr parameter, QDoubleSpinBox * spinbox)
{
	if (!parameter || !spinbox)
	{
		throw std::runtime_error("connectParameter() - Invalid parameter(s) passed!");
	}
	spinbox->setRange(parameter->minRange(), parameter->maxRange());
	spinbox->setValue(parameter->value());
	QObject::connect(spinbox, SIGNAL(valueChanged(double)), parameter.get(), SLOT(setValue(double)));
	QObject::connect(parameter.get(), SIGNAL(valueChanged(double)), spinbox, SLOT(setValue(double)));
}

void connect(NodeRanged::SPtr parameter, QAbstractButton * button)
{
	if (!parameter || !button)
	{
		throw std::runtime_error("connectParameter() - Invalid parameter(s) passed!");
	}
	button->clicked(parameter->value());
	QObject::connect(button, SIGNAL(pressed()), parameter.get(), SLOT(setOn()));
	QObject::connect(button, SIGNAL(released()), parameter.get(), SLOT(setOff()));
	QObject::connect(parameter.get(), SIGNAL(valueChanged(bool)), button, SIGNAL(clicked(bool)));
}

void connect(NodeRanged::SPtr parameter, QAction * action)
{
	if (!parameter || !action)
	{
		throw std::runtime_error("connectParameter() - Invalid parameter(s) passed!");
	}
	action->setChecked(parameter->value());
	QObject::connect(action, SIGNAL(toggled(bool)), parameter.get(), SLOT(setValue(bool)));
	QObject::connect(parameter.get(), SIGNAL(valueChanged(bool)), action, SLOT(setChecked(bool)));
}

//-------------------------------------------------------------------------------------------------

void connectParameter(ParameterBool parameter, QAbstractSlider * slider)
{
	connect(parameter.GetSharedParameter(), slider);
}

void connectParameter(ParameterInt parameter, QAbstractSlider * slider)
{
	connect(parameter.GetSharedParameter(), slider);
}

void connectParameter(ParameterFloat parameter, QAbstractSlider * slider)
{
	connect(parameter.GetSharedParameter(), slider);
}

void connectParameter(ParameterDouble parameter, QAbstractSlider * slider)
{
	connect(parameter.GetSharedParameter(), slider);
}

void connectParameter(ParameterBool parameter, QSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterInt parameter, QSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterFloat parameter, QSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterDouble parameter, QSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterBool parameter, QDoubleSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterInt parameter, QDoubleSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterFloat parameter, QDoubleSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterDouble parameter, QDoubleSpinBox * spinbox)
{
	connect(parameter.GetSharedParameter(), spinbox);
}

void connectParameter(ParameterBool parameter, QAbstractButton * button)
{
	connect(parameter.GetSharedParameter(), button);
}

void connectParameter(ParameterBool parameter, QAction * action)
{
	connect(parameter.GetSharedParameter(), action);
}
