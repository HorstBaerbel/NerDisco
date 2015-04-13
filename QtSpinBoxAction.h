#pragma once

#include <QWidgetAction>
#include <QSpinBox>


class QtSpinBoxAction : public QWidgetAction
{
	Q_OBJECT

public:
	QtSpinBoxAction(QObject * parent = NULL);
	QtSpinBoxAction(const QString & preText, QObject * parent = NULL);
	QtSpinBoxAction(const QString & preText, const QString & postText, QObject * parent = NULL);

	QSpinBox * control();

private:
	QSpinBox * m_spinBox;
};
