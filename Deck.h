#pragma once

#include "LiveView.h"
#include "CodeEdit.h"
#include "Parameters.h"
#include "MIDIInterface.h"

#include <QWidget>
#include <QTimer>
#include <QTime>

namespace Ui { class CodeDeck; }

class Deck : public QWidget
{
    Q_OBJECT

public:
    explicit Deck(QWidget *parent = 0);

	/// @brief Save the current settings to an XML document.
	/// @param parent The paren element to write the settings to.
	void toXML(QDomElement & parent) const;
	/// @brief Read current settings from XML document.
	/// @param parent The parent element to load the settings from.
	Deck & fromXML(const QDomElement & parent);

	void setDeckName(const QString & name);

	ParameterInt updateInterval;
	ParameterInt previewWidth;
	ParameterInt previewHeight;

	ParameterInt valueA;
	ParameterInt valueB;
	ParameterInt valueC;
	ParameterInt valueD;
	ParameterBool triggerA;
	ParameterBool triggerB;

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

private slots:
	void setUpdateInterval(int interval);
	void setPreviewWidth(int width);
	void setPreviewHeight(int height);
	void parameterChanged(NodeBase * parameter);

    void scriptModified(bool modified);
    void scriptTextChanged();
    void updateScriptFromText();

	void scriptCompiledOk();
	void scriptHasErrors(const QString & errors);

	void updateScriptValues();
    void updateTime();

private:
    Ui::CodeDeck *ui;
    LiveView * m_liveView;
    CodeEdit * m_codeEdit;
    QTimer m_updateTimer;
    QTime m_scriptTime;

    QString m_currentText;
	bool m_scriptModified;
    QTimer m_editTimer;
	QRegExp m_errorExp;
    QString m_currentScriptPath;

	MIDIInterface::SPtr m_midiInterface;
};
