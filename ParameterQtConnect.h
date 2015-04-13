#pragma once

#include "Parameters.h"

#include <QPushButton>
#include <QDial>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QAction>


void connectParameter(ParameterBool parameter, QAbstractSlider * slider);
void connectParameter(ParameterInt parameter, QAbstractSlider * slider);
void connectParameter(ParameterFloat parameter, QAbstractSlider * slider);
void connectParameter(ParameterDouble parameter, QAbstractSlider * slider);

void connectParameter(ParameterBool parameter, QSpinBox * spinbox);
void connectParameter(ParameterInt parameter, QSpinBox * spinbox);
void connectParameter(ParameterFloat parameter, QSpinBox * spinbox);
void connectParameter(ParameterDouble parameter, QSpinBox * spinbox);

void connectParameter(ParameterBool parameter, QDoubleSpinBox * spinbox);
void connectParameter(ParameterInt parameter, QDoubleSpinBox * spinbox);
void connectParameter(ParameterFloat parameter, QDoubleSpinBox * spinbox);
void connectParameter(ParameterDouble parameter, QDoubleSpinBox * spinbox);

void connectParameter(ParameterBool parameter, QAbstractButton * button);
void connectParameter(ParameterBool parameter, QAction * action);
