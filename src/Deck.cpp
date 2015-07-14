#include "Deck.h"
#include "ui_Deck.h"
#include "ParameterQtConnect.h"

#include <QFileDialog>
#include <QMessageBox>


Deck::Deck(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::CodeDeck)
	, m_codeEdit(new CodeEdit())
	, m_scriptTime(QTime::currentTime())
	, m_scriptModified(false)
	, m_errorExp("\\b(ERROR|Error|error)\\b:\\s?(\\d+):\\s?(\\d+):\\s?(.*)\\n")
	, m_errorExp2("\\s?(\\d+):(\\d+)\\(\\d+\\):\\s?(ERROR|Error|error):\\s?(.*)\\n")
	, m_midiInterface(MIDIInterface::getInstance())
	, updateInterval("updateInterval", 50, 20, 100)
	, asynchronousCompilation("asynchronousCompilation", false)
	, frameBufferWidth("frameBufferWidth", 128, 32, 1024)
	, frameBufferHeight("frameBufferHeight", 72, 32, 1024)
	, valueA("valueA", 0, 0, 100)
	, valueB("valueB", 0, 0, 100)
	, valueC("valueC", 0, 0, 100)
	, valueD("valueD", 0, 0, 100)
	, triggerA("triggerA", false)
	, triggerB("triggerB", false)
{
    ui->setupUi(this);
	QVBoxLayout * deckLayout = (QVBoxLayout*)ui->groupBox->layout();
    //insert code editor
	//m_codeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	deckLayout->insertWidget(0, m_codeEdit);
	//deckLayout->setStretchFactor(m_codeEdit, 1);
    //insert live editor
	QSurfaceFormat::setDefaultFormat(LiveView::getDefaultFormat());
	m_liveView = new LiveView(this);
	m_liveView->setRenderSize(frameBufferWidth, frameBufferHeight);
	m_liveView->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	QHBoxLayout * centerLayout = new QHBoxLayout();
	deckLayout->insertLayout(1, centerLayout);
	centerLayout->addWidget(m_liveView);
    //connect signals from dials to parameters
	connectParameter(valueA, ui->valueA);
	connectParameter(valueB, ui->valueB);
	connectParameter(valueC, ui->valueC);
	connectParameter(valueD, ui->valueD);
	connectParameter(triggerA, ui->triggerA);
	connectParameter(triggerB, ui->triggerB);
	//connect other parameters to functions
	connect(updateInterval.GetSharedParameter().get(), SIGNAL(valueChanged(int)), this, SLOT(setUpdateInterval(int)));
	connect(asynchronousCompilation.GetSharedParameter().get(), SIGNAL(valueChanged(bool)), m_liveView, SLOT(enableAsynchronousCompilation(bool)));
	connect(frameBufferWidth.GetSharedParameter().get(), SIGNAL(valueChanged(int)), this, SLOT(setFrameBufferWidth(int)));
	connect(frameBufferHeight.GetSharedParameter().get(), SIGNAL(valueChanged(int)), this, SLOT(setFrameBufferHeight(int)));
	//register parameters in MIDI interface
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueA.GetSharedParameter());
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueB.GetSharedParameter());
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueC.GetSharedParameter());
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueD.GetSharedParameter());
	m_midiInterface->getParameterMapping()->registerMIDIParameter(triggerA.GetSharedParameter());
	m_midiInterface->getParameterMapping()->registerMIDIParameter(triggerB.GetSharedParameter());
	//set up regular expression for error parsing
	m_errorExp.setMinimal(true);
	m_errorExp2.setMinimal(true);
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
//    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateTime()));
//	m_updateTimer.start(updateInterval);
	//load default script
	loadScript(":/effects/default.fs");
    //reset elapsed time
    m_scriptTime.start();
}

Deck::~Deck()
{
}

void Deck::toXML(QDomElement & parent) const
{
	//try to find element in document
	QDomNodeList children = parent.elementsByTagName("Deck");
	for (int i = 0; i < children.size(); ++i)
	{
		QDomElement child = children.at(i).toElement();
		if (!child.isNull() && child.attribute("name") == objectName())
		{
			//remove child from document, we'll re-add it
			parent.removeChild(child);
			break;
		}
	}
	//(re)add the new element
	QDomElement element = parent.ownerDocument().createElement("Deck");
	element.setAttribute("name", objectName());
	element.setAttribute("currentScriptPath", m_currentScriptPath);
	element.setAttribute("scriptModified", m_scriptModified);
	//if the script hasn't been modified, do not store the text, because it is identical to the file
	element.setAttribute("currentText", m_scriptModified ? m_currentText : "");
	updateInterval.toXML(element);
	asynchronousCompilation.toXML(element);
	valueA.toXML(element);
	valueB.toXML(element);
	valueC.toXML(element);
	valueD.toXML(element);
	triggerA.toXML(element);
	triggerB.toXML(element);
	parent.appendChild(element);
}

Deck & Deck::fromXML(const QDomElement & parent)
{
	//try to find element in document
	QDomNodeList decks = parent.elementsByTagName("Deck");
	if (decks.isEmpty())
	{
		throw std::runtime_error("No deck settings found!");
	}
	//try to find object name in children
	for (int j = 0; j < decks.size(); ++j)
	{
		QDomElement child = decks.at(j).toElement();
		if (!child.isNull() && child.attribute("name") == objectName())
		{
			//found. read and apply settings
 			loadScript(child.attribute("currentScriptPath"));
			scriptModified(child.attribute("scriptModified", "0").toUInt());
			//only read text if the script has been modified, else it is the same as the file already loaded...
			if (m_scriptModified)
			{
				m_codeEdit->setPlainText(child.attribute("currentText"));
			}
			updateInterval.fromXML(child);
			asynchronousCompilation.fromXML(child);
			valueA.fromXML(child);
			valueB.fromXML(child);
			valueC.fromXML(child);
			valueD.fromXML(child);
			triggerA.fromXML(child);
			triggerB.fromXML(child);
			return *this;
		}
	}
	throw std::runtime_error("No settings found for deck!");
}

void Deck::setDeckName(const QString & name)
{
	setObjectName(name);
	//register parameters in MIDI interface
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueA.GetSharedParameter(), name);
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueB.GetSharedParameter(), name);
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueC.GetSharedParameter(), name);
	m_midiInterface->getParameterMapping()->registerMIDIParameter(valueD.GetSharedParameter(), name);
	m_midiInterface->getParameterMapping()->registerMIDIParameter(triggerA.GetSharedParameter(), name);
	m_midiInterface->getParameterMapping()->registerMIDIParameter(triggerB.GetSharedParameter(), name);
}

void Deck::setUpdateInterval(int interval)
{
	m_updateTimer.setInterval(interval);
	updateInterval = interval;
}

void Deck::setFrameBufferWidth(int width)
{
	frameBufferWidth = width;
	m_liveView->setRenderSize(frameBufferWidth, frameBufferHeight);
}

void Deck::setFrameBufferHeight(int height)
{
	frameBufferHeight = height;
	m_liveView->setRenderSize(frameBufferWidth, frameBufferHeight);
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
		ui->groupBox->setTitle(objectName() + " (" + m_currentScriptPath + ")");
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
				m_scriptModified = false;
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
				m_scriptModified = false;
                m_codeEdit->document()->setModified(false);
                //make path relative
                m_currentScriptPath = QDir::current().relativeFilePath(filePath);
				ui->groupBox->setTitle(objectName() + " (" + m_currentScriptPath + ")");
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
    m_editTimer.start(1000);
}

void Deck::scriptModified(bool modified)
{
	m_scriptModified = modified;
	ui->groupBox->setTitle(objectName() + " (" + m_currentScriptPath + ")" + (m_scriptModified ? "*" : ""));
}

void Deck::scriptCompiledOk()
{
	m_codeEdit->setErrors();
	updateScriptValues();
}

void Deck::scriptHasErrors(const QString & errors)
{
	//figure out line number adjustment due to script prefix
	const int prefixLineCount = m_liveView->currentScriptPrefix().count(QLatin1Char('\n'));
	//parse errors
	QVector<CodeEdit::Error> list;
	CodeEdit::Error error;
	error.line = 0;
	error.column = 0;
	error.message = "Unknown script error";
	if (m_errorExp.indexIn(errors) >= 0)
	{
		error.line = m_errorExp.cap(3).toInt() + 1 - prefixLineCount;
		error.column = 0;
		error.message = m_errorExp.cap(4);
	}
	else if (m_errorExp2.indexIn(errors) >= 0)
	{
		error.line = m_errorExp2.cap(2).toInt() + 1 - prefixLineCount;
		error.column = 0;
		error.message = m_errorExp2.cap(4);
	}
	list.append(error);
	m_codeEdit->setErrors(list);
}

void Deck::updateScriptValues()
{
    //update properties in new active script
    m_liveView->setFragmentScriptProperty("time", (float)m_scriptTime.elapsed() / 1000.0f);
	m_liveView->setFragmentScriptProperty(valueA.name(), valueA.normalizedValue());
	m_liveView->setFragmentScriptProperty(valueB.name(), valueB.normalizedValue());
	m_liveView->setFragmentScriptProperty(valueC.name(), valueC.normalizedValue());
	m_liveView->setFragmentScriptProperty(valueD.name(), valueD.normalizedValue());
	m_liveView->setFragmentScriptProperty(triggerA.name(), triggerA.normalizedValue());
	m_liveView->setFragmentScriptProperty(triggerB.name(), triggerB.normalizedValue());
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

void Deck::parameterChanged(NodeBase * parameter)
{
	if (dynamic_cast<ParameterBool*>(parameter))
	{
		ParameterBool * p = dynamic_cast<ParameterBool*>(parameter);
		m_liveView->setFragmentScriptProperty(p->name(), *p);
	}
	else if (dynamic_cast<ParameterInt*>(parameter))
	{
		ParameterInt * p = dynamic_cast<ParameterInt*>(parameter);
		m_liveView->setFragmentScriptProperty(p->name(), p->normalizedValue());
	}
	else if (dynamic_cast<ParameterFloat*>(parameter))
	{
		ParameterFloat * p = dynamic_cast<ParameterFloat*>(parameter);
		m_liveView->setFragmentScriptProperty(p->name(), p->normalizedValue());
	}
}
