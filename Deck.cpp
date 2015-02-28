#include "Deck.h"
#include "ui_Deck.h"
#include "Settings.h"

#include <QFileDialog>
#include <QMessageBox>


Deck::Deck(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeDeck)
    , m_liveView(new LiveView())
    , m_codeEdit(new CodeEdit())
    , m_scriptTime(QTime::currentTime())
	, m_errorExp("\\b(ERROR|Error|error)\\b:\\s?(\\d+):\\s?(\\d+):\\s?(.*)\\n")
{
    ui->setupUi(this);
    m_deckName = ui->groupBox->title();
    //insert code editor
    ((QVBoxLayout*)ui->groupBox->layout())->insertWidget(0, m_codeEdit);
    //insert live editor
    QHBoxLayout * viewLayout = new QHBoxLayout;
    viewLayout->setMargin(0);
    viewLayout->setSpacing(0);
    ((QVBoxLayout*)ui->groupBox->layout())->insertLayout(1, viewLayout);
    viewLayout->addWidget(m_liveView);
    //connect signals from dials
    connect(ui->dialA, SIGNAL(valueChanged(int)), this, SLOT(valueAChanged(int)));
    connect(ui->dialB, SIGNAL(valueChanged(int)), this, SLOT(valueBChanged(int)));
    connect(ui->dialC, SIGNAL(valueChanged(int)), this, SLOT(valueCChanged(int)));
    connect(ui->pushButtonTrigger, SIGNAL(pressed()), this, SLOT(triggerPressed()));
    connect(ui->pushButtonTrigger, SIGNAL(released()), this, SLOT(triggerReleased()));
	//set up regular expression for error parsing
	m_errorExp.setMinimal(true);
    //when the script changes either sucessfully or has errors, we get notified
	connect(m_liveView, SIGNAL(fragmentScriptChanged()), this, SLOT(scriptCompiledOk()));
	connect(m_liveView, SIGNAL(fragmentScriptErrors(const QString &)), this, SLOT(scriptHasErrors(const QString &)));
	connect(m_liveView, SIGNAL(renderingFinished()), this, SIGNAL(renderingFinished()));
    //when the script is being modified, we keep track of that in the GUI
    connect(m_codeEdit->document(), SIGNAL(modificationChanged(bool)), this, SLOT(scriptModified(bool)));
    //when the script is edited a timer is started to wait a bit before updating the script
    connect(m_codeEdit, SIGNAL(textChanged()), this, SLOT(scriptTextChanged()));
    //set up timer that waits while the user edits the document
    connect(&m_editTimer, SIGNAL(timeout()), this, SLOT(updateScriptFromText()));
    m_editTimer.setSingleShot(true);
    //set up timer for updating the time property
    Settings & settings = Settings::getInstance();
    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateTime()));
    m_updateTimer.start(settings.displayInterval());
    //reset elapsed time
    m_scriptTime.start();
    //load default script
    loadScript(":/effects/default.fs");
}

Deck::~Deck()
{
}

bool Deck::loadScript(const QString & path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
        //set script in editor. compilation will run automatically
        m_codeEdit->setPlainText(file.readAll());
        m_codeEdit->document()->setModified(false);
        m_currentScriptPath = path;
        if (!m_currentScriptPath.startsWith(":/"))
        {
            m_currentScriptPath = QDir::current().relativeFilePath(path);
        }
        ui->groupBox->setTitle(m_deckName + " (" + m_currentScriptPath + ")");
        return true;
    }
    return false;
}

bool Deck::saveScript()
{
    //check if the script name is not empty and the script is not coming from a resource
    if (!m_currentScriptPath.isEmpty() && !m_currentScriptPath.startsWith(":/"))
    {
        QFile file(m_currentScriptPath);
        if (file.open(QFile::WriteOnly))
        {
            QByteArray data(m_codeEdit->toPlainText().toUtf8());
            if (file.write(data) == data.size())
            {
                m_codeEdit->document()->setModified(false);
                return true;
            }
        }
        else
        {
            QMessageBox::StandardButton result = QMessageBox::warning(this, tr("Failed to open file!") , tr("Failed to open \"") + m_currentScriptPath + tr("\". Do you want to save the script under a different name?"), QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::No)
            {
                return false;
            }
        }
    }
    return saveAsScript();
}

bool Deck::saveAsScript(const QString & path)
{
    QString filePath = path;
    //if the path is empty, query the user for a file name
    if (filePath.isEmpty())
    {
        QString startName = m_currentScriptPath.startsWith(":/") ? QFileInfo(m_currentScriptPath).fileName() : m_currentScriptPath;
        filePath = QFileDialog::getSaveFileName(this, tr("Save current script as"), startName, tr("Fragment shader files (*.fs)"));
    }
    if (!filePath.isEmpty())
    {
        QFile file(filePath);
        if (file.open(QFile::WriteOnly))
        {
            QByteArray data(m_codeEdit->toPlainText().toUtf8());
            if (file.write(data) == data.size())
            {
                m_codeEdit->document()->setModified(false);
                //make path relative
                m_currentScriptPath = QDir::current().relativeFilePath(filePath);
                ui->groupBox->setTitle(m_deckName + " (" + m_currentScriptPath + ")");
                return true;
            }
        }
    }
    return false;
}

void Deck::updateScriptFromText()
{
    if (m_currentText != m_codeEdit->toPlainText())
    {
        //store new current text
        m_currentText = m_codeEdit->toPlainText();
		//send script to live view to compile it
		m_liveView->setFragmentScript(m_currentText.toLocal8Bit());
    }
}

void Deck::scriptTextChanged()
{
    //user has edited the text. stop timer and start it again to wait a second before updating the script
    m_editTimer.stop();
    m_editTimer.start(500);
}

void Deck::scriptModified(bool modified)
{
    ui->groupBox->setTitle(m_deckName + " (" + m_currentScriptPath + ")" + (modified ? "*" : ""));
}

void Deck::scriptCompiledOk()
{
	m_codeEdit->setErrors();
	updateScriptValues();
}

void Deck::scriptHasErrors(const QString & errors)
{
	//parse errors
	QVector<CodeEdit::Error> list;
	CodeEdit::Error error;
	error.line = 0;
	error.column = 0;
	error.message = "Unknown script error";
	if (m_errorExp.indexIn(errors) >= 0)
	{
		error.line = m_errorExp.cap(3).toInt() + 1;
		error.column = 0;
		error.message = m_errorExp.cap(4);
	}
	list.append(error);
	m_codeEdit->setErrors(list);
}

void Deck::updateScriptValues()
{
    //update properties in new active script
    m_liveView->setFragmentScriptProperty("time", (float)m_scriptTime.elapsed() / 1000.0f);
	m_liveView->setFragmentScriptProperty("valueA", (float)ui->dialA->value() / 100.0f);
	m_liveView->setFragmentScriptProperty("valueB", (float)ui->dialB->value() / 100.0f);
	m_liveView->setFragmentScriptProperty("valueC", (float)ui->dialC->value() / 100.0f);
	m_liveView->setFragmentScriptProperty("trigger", ui->pushButtonTrigger->isDown());
}

void Deck::render()
{
	updateScriptValues();
	m_liveView->render();
}

void Deck::grabFramebufferAfterSwap()
{
	m_liveView->grabFramebufferAfterSwap();
}

QImage Deck::getGrabbedFramebuffer()
{
	return m_liveView->getGrabbedFramebuffer();
}

void Deck::updateTime()
{
	m_liveView->setFragmentScriptProperty("time", (float)m_scriptTime.elapsed() / 1000.0f);
}

void Deck::setValue(const QString & controlName, float value)
{
	if (controlName == ui->dialA->objectName())
	{
		if (ui->dialA->value() != value * 100.0f)
		{
			ui->dialA->setValue(value * 100.0f);
		}
	}
	else if (controlName == ui->dialB->objectName())
	{
		if (ui->dialB->value() != value * 100.0f)
		{
			ui->dialB->setValue(value * 100.0f);
		}
	}
	else if (controlName == ui->dialC->objectName())
	{
		if (ui->dialC->value() != value * 100.0f)
		{
			ui->dialC->setValue(value * 100.0f);
		}
	}
	else if (controlName == ui->pushButtonTrigger->objectName())
	{
		if (value <= 0.0f)
		{
			triggerReleased();
		}
		else
		{
			triggerPressed();
		}
		ui->pushButtonTrigger->setDown(value > 0.0f);
	}
}

void Deck::valueAChanged(int value)
{
	const float fvalue = (float)value / 100.0;
	m_liveView->setFragmentScriptProperty("valueA", fvalue);
	emit valueChanged(ui->dialA->objectName(), fvalue);
}

void Deck::valueBChanged(int value)
{
	const float fvalue = (float)value / 100.0;
	m_liveView->setFragmentScriptProperty("valueB", fvalue);
	emit valueChanged(ui->dialB->objectName(), fvalue);
}

void Deck::valueCChanged(int value)
{
	const float fvalue = (float)value / 100.0;
	m_liveView->setFragmentScriptProperty("valueC", fvalue);
	emit valueChanged(ui->dialC->objectName(), fvalue);
}

void Deck::triggerPressed()
{
	m_liveView->setFragmentScriptProperty("trigger", 1.0f);
	emit valueChanged(ui->pushButtonTrigger->objectName(), 1.0f);
}

void Deck::triggerReleased()
{
	m_liveView->setFragmentScriptProperty("trigger", 0.0f);
	emit valueChanged(ui->pushButtonTrigger->objectName(), 0.0f);
}
