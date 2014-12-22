#include "deck.h"
#include "ui_codedeck.h"
#include "settings.h"

#include <QtQml/QQmlContext>


Deck::Deck(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CodeDeck)
    , m_liveView(new LiveView())
    , m_codeEdit(new CodeEdit())
    , m_scriptComponent(NULL)
    , m_scriptTime(QTime::currentTime())
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
    //when the script in the QML changes, we get notified, so we can update its values
    connect(m_liveView, SIGNAL(renderScriptChanged()), this, SLOT(updateScriptValues()));
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
    m_updateTimer.start(settings.displayInterval()*0.95);
    //reset elapsed time
    m_scriptTime.start();
    //load default script
    loadScript(":/effects/default.qml");
}

Deck::~Deck()
{
    //delete the component again
    delete m_scriptComponent;
    //delete GUI
    delete ui;
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
        ui->groupBox->setTitle(m_deckName + " (" + path + ")");
        return true;
    }
    return false;
}

bool Deck::saveScript()
{
    if (!m_currentScriptPath.isEmpty())
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
    }
    return false;
}

bool Deck::saveAsScript(const QString & path)
{
    if (!path.isEmpty())
    {
        QFile file(path);
        if (file.open(QFile::WriteOnly))
        {
            QByteArray data(m_codeEdit->toPlainText().toUtf8());
            if (file.write(data) == data.size())
            {
                m_codeEdit->document()->setModified(false);
                m_currentScriptPath = path;
                ui->groupBox->setTitle(m_deckName + " (" + path + ")");
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
        //create component if needed
        if (m_scriptComponent == NULL)
        {
            m_scriptComponent = new QQmlComponent(m_liveView->quickView()->engine());
        }
        //try to compile the text to QML component and check errors
        m_scriptComponent->setData("import QtQuick 2.0\n" + m_currentText.toUtf8(), QUrl());
        QList<QQmlError> scriptErrors = m_scriptComponent->errors();
        //free QML component. we will re-create it later, because setData does not seem to work twice...
        delete m_scriptComponent;
        m_scriptComponent = NULL;
        //check if any errors occured
        if (scriptErrors.size() > 0)
        {
            //convert errors so we can display them in code editor
            QVector<CodeEdit::Error> errors;
            foreach(const QQmlError & error, scriptErrors)
            {
                CodeEdit::Error newError;
                newError.line = error.line() - 1; //we subtract 1 because we added "import QtQuick 2.0\n" at the start...
                newError.column = error.column();
                newError.message = error.description();
                errors.append(newError);
            }
            //send to code editor
            m_codeEdit->setErrors(errors);
        }
        else
        {
            //send script text to the view
            m_liveView->setRenderScript(m_currentText);
            //clear errors in code editor
            m_codeEdit->setErrors(QVector<CodeEdit::Error>());
        }
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

void Deck::updateScriptValues()
{
    //update properties in new active script
    m_liveView->setRenderScriptProperty("time", (double)m_scriptTime.elapsed() / 1000.0);
    m_liveView->setRenderScriptProperty("valueA", (double)ui->dialA->value() / 100.0);
    m_liveView->setRenderScriptProperty("valueB", (double)ui->dialB->value() / 100.0);
    m_liveView->setRenderScriptProperty("valueC", (double)ui->dialC->value() / 100.0);
    m_liveView->setRenderScriptProperty("trigger", ui->pushButtonTrigger->isDown());
}

QImage Deck::grabFramebuffer()
{
    return m_liveView->grabFramebuffer();
}

void Deck::updateTime()
{
    m_liveView->setRenderScriptProperty("time", (double)m_scriptTime.elapsed() / 1000.0);
}

void Deck::valueAChanged(int value)
{
    m_liveView->setRenderScriptProperty("valueA", (double)value / 100.0);
}

void Deck::valueBChanged(int value)
{
    m_liveView->setRenderScriptProperty("valueB", (double)value / 100.0);
}

void Deck::valueCChanged(int value)
{
    m_liveView->setRenderScriptProperty("valueC", (double)value / 100.0);
}

void Deck::triggerPressed()
{
    m_liveView->setRenderScriptProperty("trigger", true);
}

void Deck::triggerReleased()
{
    m_liveView->setRenderScriptProperty("trigger", false);
}
