#pragma once

#include <QOpenGLContext>
#include <QOpenGLShader>
#include <QOffscreenSurface>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>


class GLSLCompileThread : public QThread
{
	Q_OBJECT

public:
	/// @brief Constructor.
	/// @param shareContext Non-NULL context to share.
	/// @param parent Parent object.
	GLSLCompileThread(QOpenGLContext * shareContext, QObject * parent = nullptr);

	~GLSLCompileThread();

public slots:
	/// @brief Try to asynchronously compile a GLSL shader script. Will emit a result() when done.
	/// @param vertexCode Vertex shader code string.
	/// @param fragmentCode Fragment shader code string.
	/// @param asynchronous Pass true to do synchronous compilation.
	void compileAndLink(QString vertexCode, QString fragmentCode, bool asynchronous = false);

signals:
	/// @brief Emitted when compilation has finished and either succeeded or failed.
	/// @param vertex New vertex shader if compilation succeeded, or NULL if compilation failed. The receiver has to take ownership of the object!
	/// @param fragment New vertex shader if compilation succeeded, or NULL if compilation failed. The receiver has to take ownership of the object!
	/// @param program New shader program if compilation succeeded, or NULL if compilation failed. The receiver has to take ownership of the object!
	/// @param success True if compilation succeeded.
	/// @param errors Error string from shader compilation.
	void result(QOpenGLShader * vertex, QOpenGLShader * fragment, QOpenGLShaderProgram * program, bool success, const QString & errors);

protected:
	void run();

	void compileInternal(QString vertexCode, QString fragmentCode);

private:
	QMutex m_mutex;
	QWaitCondition m_condition;
	bool m_quit;
	QOpenGLContext * m_context;
	QOffscreenSurface * m_surface;
	QString m_vertexCode;
	QString m_fragmentCode;
};
