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
	ParameterBool asynchronousCompilation;
	ParameterInt frameBufferWidth;
	ParameterInt frameBufferHeight;

	ParameterInt valueA;
	ParameterInt valueB;
	ParameterInt valueC;
	ParameterInt valueD;
	ParameterBool triggerA;
	ParameterBool triggerB;

	ParameterBool autoCycleScripts;
	ParameterInt autoCycleInterval;

	void setScriptPath(const QString & scriptPath);
    bool loadScript(const QString & path);
    bool saveScript();
    bool saveAsScript(const QString & path = "");
	static QStringList buildScriptList(const QString & path);

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
	void setAutoCycleScripts(bool enable);
	void setAutoCycleInterval(int seconds);
	void loadNextScript();

	void setUpdateInterval(int interval);
	void setFrameBufferWidth(int width);
	void setFrameBufferHeight(int height);
	void parameterChanged(NodeBase * parameter);

    void scriptModified(bool modified);
    void scriptTextChanged();
    void updateScriptFromText();

	void scriptCompiledOk();
	void scriptHasErrors(const QString & errors);

	void setScriptParameter(ParameterBool parameter, const QString & value);
	void setScriptParameter(ParameterInt parameter, const QString & value);
	void setScriptParameter(const QString & name, const QString & value);
	void updateScriptValues();
    void updateTime();

private:
	QRegExp m_commentExp;
	QRegExp m_errorExp;
	QRegExp m_errorExp2;

    Ui::CodeDeck *ui;
    LiveView * m_liveView;
    CodeEdit * m_codeEdit;
    QTimer m_updateTimer;
    QTime m_scriptTime;

    QString m_currentText;
	bool m_scriptModified;
    QTimer m_editTimer;
    QString m_currentScriptPath;
	QString m_scriptPath;

	QTimer m_cycleTimer;

	MIDIInterface::SPtr m_midiInterface;
};
