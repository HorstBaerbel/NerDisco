#pragma once

#include "liveview.h"
#include "CodeEdit.h"

#include <QWidget>
#include <QTimer>

namespace Ui { class CodeDeck; }

class Deck : public QWidget
{
    Q_OBJECT

public:
    explicit Deck(QWidget *parent = 0);

    bool loadScript(const QString & path);
    bool saveScript();
    bool saveAsScript(const QString & path = "");

    QImage grabFramebuffer();

    ~Deck();

private slots:
    void scriptModified(bool modified);
    void scriptTextChanged();
    void updateScriptFromText();
    void updateScriptValues();

    void updateTime();
    void valueAChanged(int value);
    void valueBChanged(int value);
    void valueCChanged(int value);
    void triggerPressed();
    void triggerReleased();

private:
    Ui::CodeDeck *ui;
    LiveView * m_liveView;
    CodeEdit * m_codeEdit;
    QTimer m_updateTimer;
    QTime m_scriptTime;

    QString m_currentText;
    QQmlComponent * m_scriptComponent;
    QTimer m_editTimer;

    QString m_deckName;
    QString m_currentScriptPath;
};
