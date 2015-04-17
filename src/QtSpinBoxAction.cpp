#include "QtSpinBoxAction.h"

#include <QHBoxLayout>
#include <QLabel>


QtSpinBoxAction::QtSpinBoxAction(QObject * parent)
	: QWidgetAction(parent)
{
	m_spinBox = new QSpinBox(NULL);
	setDefaultWidget(m_spinBox);
}

QtSpinBoxAction::QtSpinBoxAction(const QString & preText, QObject * parent)
	: QWidgetAction(parent)
{
	setCheckable(false);
	QWidget * actionWidget = new QWidget(NULL);
	QHBoxLayout * actionLayout = new QHBoxLayout();
	actionLayout->setMargin(1);
	QLabel * spinBoxLabel = new QLabel(preText);
	actionLayout->addWidget(spinBoxLabel);
	m_spinBox = new QSpinBox(NULL);
	m_spinBox->setAlignment(Qt::AlignRight);
	actionLayout->addWidget(m_spinBox);
	actionWidget->setLayout(actionLayout);
	setDefaultWidget(actionWidget);
}

QtSpinBoxAction::QtSpinBoxAction(const QString & preText, const QString & postText, QObject * parent)
	: QWidgetAction(parent)
{
	setCheckable(false);
	QWidget * actionWidget = new QWidget(NULL);
	QHBoxLayout * actionLayout = new QHBoxLayout();
	actionLayout->setMargin(1);
	if (!preText.isEmpty())
	{
		QLabel * preLabel = new QLabel(preText);
		actionLayout->addWidget(preLabel);
	}
	m_spinBox = new QSpinBox(NULL);
	m_spinBox->setAlignment(Qt::AlignRight);
	actionLayout->addWidget(m_spinBox);
	if (!postText.isEmpty())
	{
		QLabel * postLabel = new QLabel(postText);
		actionLayout->addWidget(postLabel);
	}
	actionWidget->setLayout(actionLayout);
	setDefaultWidget(actionWidget);
}

QSpinBox * QtSpinBoxAction::control()
{
	return m_spinBox;
}
