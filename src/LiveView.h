#pragma once

#include "SwapThread.h"

#include <QGLWidget>
#include <QGLFormat>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_2_1>
#include <QMap>
#include <QMutex>


class LiveView : public QGLWidget, protected QOpenGLFunctions_2_1
{
    Q_OBJECT

public:
    LiveView(QWidget * parent = NULL, QGLWidget * sharedWidget = NULL);

	/// @brief Get the default OpenGL format used for the live view.
	static QGLFormat getDefaultFormat();

	/// @brief Call when you want the framebuffer after the next buffer swap.
	/// You can retrieve the last grabbed framebuffer using QImage getGrabbedFrameBuffer().
	void grabFramebufferAfterSwap();

	/// @brief Retrieve the last grabbed framebuffer. Call void grabFrameBufferAfterSwap() to grab it after a buffer swap.
	QImage getGrabbedFramebuffer();

	/// @brief Set new render script, actually a fragment shader
    void setFragmentScript(const QString & script);

	/// @brief Set parameter in fragment shader.
	void setFragmentScriptProperty(const QString & name, const QVector2D & value);
	void setFragmentScriptProperty(const QString & name, const QVector3D & value);
	void setFragmentScriptProperty(const QString & name, const QVector4D & value);
	void setFragmentScriptProperty(const QString & name, float value);
	void setFragmentScriptProperty(const QString & name, double value);
	void setFragmentScriptProperty(const QString & name, unsigned int value);
	void setFragmentScriptProperty(const QString & name, int value);
	void setFragmentScriptProperty(const QString & name, bool value);

public slots:
	/// @brief Render the scene and send signal renderingFinished() afterwards.
	void render();

signals:
	/// @brief Called when setScript is called, but shader compilation failed.
	/// @param errors Error log from shader compilation / linking.
	void fragmentScriptErrors(const QString & errors);
	/// @brief Called when setScript is called and the script was sucessfully compiled and will be displayed.
    void fragmentScriptChanged();
	/// @brief render() was called and rendering and the asynchronous buffer swap have finished.
	void renderingFinished();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);
	virtual void paintGL();

	virtual void resizeEvent(QResizeEvent * event);
	virtual void paintEvent(QPaintEvent * event);

protected slots:
	void bufferSwapFinished();

private:
	static const float m_quadData[20];
	static const char * m_defaultVertexCode;
	static const char * m_defaultFragmentCode;

	bool m_mustInitialize;
	bool m_renderRequested;
	QMap<QString, QVector2D> m_shaderValues2d;
	QMap<QString, QVector3D> m_shaderValues3d;
	QMap<QString, QVector4D> m_shaderValues4d;
	QMap<QString, float> m_shaderValuesf;
	QMap<QString, unsigned int> m_shaderValuesui;
	QMap<QString, int> m_shaderValuesi;
	QMap<QString, bool> m_shaderValuesb;
	QOpenGLShader * m_vertexShader;
	QOpenGLShader * m_fragmentShader;
	QOpenGLShaderProgram * m_shaderProgram;
	QString m_fragmentScript;

	SwapThread * m_swapThread;

	QMutex m_grabMutex;
	bool m_grabFramebuffer;
	QImage m_grabbedFramebuffer;
};
