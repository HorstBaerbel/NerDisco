#pragma once

//#include "SwapThread.h"
#include "GLSLCompileThread.h"

#include <QMap>
#include <QMutex>
#include <QMatrix4x4>
#include <QSurfaceFormat>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>


class LiveView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
	LiveView(QWidget * parent = NULL);

	~LiveView();

	virtual bool hasHeightForWidth() const override;
	virtual int heightForWidth(int width) const override;
	virtual QSize sizeHint() const override;

	/// @brief Get the default OpenGL format used for the live view.
	static QSurfaceFormat getDefaultFormat();

	/// @brief Call when you want the framebuffer after the next buffer swap.
	/// You can retrieve the last grabbed framebuffer using QImage getGrabbedFrameBuffer().
	void grabFramebufferAfterSwap();

	/// @brief Retrieve the last grabbed framebuffer. Call void grabFrameBufferAfterSwap() to grab it after a buffer swap.
	QImage getGrabbedFramebuffer();

	/// @brief Set new render script, actually a fragment shader
    void setFragmentScript(const QString & script);

	/// @brief Retrieve current prefix applied to fragment script to make it compilable.
	/// @return Current script prefix.
	QString currentScriptPrefix() const;

	/// @brief Set parameter in fragment shader.
	void setFragmentScriptProperty(const QString & name, const QVector2D & value);
	void setFragmentScriptProperty(const QString & name, const QVector3D & value);
	void setFragmentScriptProperty(const QString & name, const QVector4D & value);
	void setFragmentScriptProperty(const QString & name, float value);
	void setFragmentScriptProperty(const QString & name, double value);
	void setFragmentScriptProperty(const QString & name, unsigned int value);
	void setFragmentScriptProperty(const QString & name, int value);
	void setFragmentScriptProperty(const QString & name, bool value);

	/// @brief Set a different size than the preview / actual widget size.
	/// This is the size the image will be rendered in. It will the be rescaled to the widget size.
	void setRenderSize(int width, int height);

public slots:
	/// @brief Toggle asynchronous shader compilation. This crashes on some systems.
	/// @param enabled Pass true to enable. Default is disabled.
	void enableAsynchronousCompilation(bool enabled = false);

	/// @brief Render the scene and send signal renderingFinished() afterwards.
	void render();

signals:
	/// @brief Called when setScript is called, but shader compilation failed.
	/// @param errors Error log from shader compilation / linking.
	/// @note The line numbers here must not exactly match the script passed.
	/// This is because a prefix has been added to the script. You can get the current prefix
	/// using currentScriptPrefix() to adjust line numbers.
	void fragmentScriptErrors(const QString & errors);
	/// @brief Called when setScript is called and the script was sucessfully compiled and will be displayed.
    void fragmentScriptChanged();
	/// @brief render() was called and rendering and the asynchronous buffer swap have finished.
	void renderingFinished();

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int width, int height) override;
	virtual void paintGL() override;

	//virtual void resizeEvent(QResizeEvent * event) override;
	virtual void paintEvent(QPaintEvent * event) override;

protected slots:
	void bufferSwapFinished();
	void compilationFinished(QOpenGLShader * vertex, QOpenGLShader * fragment, QOpenGLShaderProgram * program, bool success, const QString & errors);

private:
	void CreateFrameBufferShader();
	void CreateFrameBuffer();

	static const float m_quadData[20];
	static const char * m_vertexPrefixGLES2;
	static const char * m_fragmentPrefixGLES2;
	static const char * m_vertexPrefixGL2;
	static const char * m_fragmentPrefixGL2;
	static const char * m_defaultVertexCode;
	static const char * m_defaultFragmentCode;
	static const char * m_frameBufferFragmentCode;
	QString m_vertexPrefix;
	QString m_fragmentPrefix;

	bool m_mustInitialize;
	bool m_renderRequested;

	int m_frameBufferWidth;
	int m_frameBufferHeight;
	bool m_keepAspect;
	QOpenGLFramebufferObject * m_frameBufferObject;
	QOpenGLShader * m_frameBufferVertexShader;
	QOpenGLShader * m_frameBufferFragmentShader;
	QOpenGLShaderProgram * m_frameBufferShaderProgram;
	QMatrix4x4 m_blitMatrix;

	QMatrix4x4 m_projectionMatrix;
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
	bool m_scriptChanged;

	//SwapThread * m_swapThread;
	GLSLCompileThread * m_compileThread;
	bool m_asynchronousCompilation;

	QMutex m_grabMutex;
	bool m_grabFramebuffer;
	QImage m_grabbedFramebuffer;
};
