#pragma once

#include "LiveView.h"
#include "CodeEdit.h"

#include <QWidget>
#include <QTimer>
#include <QTime>

namespace Ui { class CodeDeck; }

class Deck : public QWidget
{
    Q_OBJECT

public:
    explicit Deck(QWidget *parent = 0);

    bool loadScript(const QString & path);
    bool saveScript();
    bool saveAsScript(const QString & path = "");

	/// @brief Update view and emit signal renderingFinished when rendering and the asynchronous buffer swap have finished.
	void render();

	/// @brief Call when you want the framebuffer after the next buffer swap.
	/// You can retrieve the last grabbed framebuffer using QImage getGrabbedFrameBuffer().
	void grabFramebufferAfterSwap();

	/// @brief Retrieve the last grabbed framebuffer. Call void grabFrameBufferAfterSwap() to grab it after a buffer swap.
	QImage getGrabbedFramebuffer();

    ~Deck();

signals:
	/// @brief render() was called and rendering and the asynchronous buffer swap have finished.
	void renderingFinished();
	/// @brief The value of a control in the deck has changed and the new value is sent.
	/// @param controlName Name of control that has changed.
	/// @param value New value in the range [0,1].
	/// @note The name of the control can be used in void setValue(const QString & which, float value) to set a value.
	void valueChanged(const QString & controlName, float value);

public slots:
	/// @brief Change the value of a control in the deck.
	/// @param controlName Name of control to change.
	/// @param value New value in the range [0,1].
	/// @note The name of the control is tha same as in the signal void valueChanged(const QString & controlName, float value).
	void setValue(const QString & controlName, float value);

private slots:
    void scriptModified(bool modified);
    void scriptTextChanged();
    void updateScriptFromText();

	void scriptCompiledOk();
	void scriptHasErrors(const QString & errors);

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
    QTimer m_editTimer;
	QRegExp m_errorExp;

    QString m_deckName;
    QString m_currentScriptPath;
};
