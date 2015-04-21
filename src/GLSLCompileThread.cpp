#include "GLSLCompileThread.h"


GLSLCompileThread::GLSLCompileThread(QOpenGLContext * shareContext, QObject * parent)
	: QThread(parent)
	, m_context(nullptr)
	, m_surface(nullptr)
	, m_quit(false)
{
	if (shareContext == nullptr)
	{
		throw std::runtime_error("GLSLCompileThread::GLSLCompileThread() - NULL context passed!");
	}
	//allocate shared context
	m_context = new QOpenGLContext();
	m_context->setFormat(shareContext->format());
	m_context->setShareContext(shareContext);
	m_context->moveToThread(this);
	m_context->create();
	//do not create the context now, but later in run()
	//create invisible surface to make context current
	m_surface = new QOffscreenSurface();
	m_surface->setFormat(shareContext->format());  
	m_surface->create();
}

GLSLCompileThread::~GLSLCompileThread()
{
	m_quit = true;
	m_condition.wakeAll();
	wait();
	delete m_context;
	delete m_surface;
}

void GLSLCompileThread::compileAndLink(QString vertexCode, QString fragmentCode, bool asynchronous)
{
	QMutexLocker locker(&m_mutex);
	//check if we want asynchronous compilation
	if (asynchronous)
	{
		m_vertexCode = vertexCode;
		m_fragmentCode = fragmentCode;
		if (!isRunning())
		{
			start();
		}
		else
		{
			m_condition.wakeOne();
		}
	}
	else
	{
		compileInternal(vertexCode, fragmentCode);
	}
}

void GLSLCompileThread::compileInternal(QString vertexCode, QString fragmentCode)
{
	//try to compile the fragment shader
	QString errors;
	QOpenGLShader * fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
	if (!fragmentShader->compileSourceCode(fragmentCode))
	{
		errors.append(fragmentShader->log());
		delete fragmentShader;
		emit result(nullptr, nullptr, nullptr, false, errors);
	}
	else
	{
		//fragment shader is ik. try compiling vertex shader
		QOpenGLShader * vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
		if (!vertexShader->compileSourceCode(vertexCode))
		{
			errors.append(vertexShader->log());
			delete fragmentShader;
			delete vertexShader;
			emit result(nullptr, nullptr, nullptr, false, errors);
		}
		else
		{
			//compilation worked. try linking
			QOpenGLShaderProgram * program = new QOpenGLShaderProgram();
			program->addShader(vertexShader);
			program->addShader(fragmentShader);
			if (!program->link())
			{
				errors.append(program->log());
				delete fragmentShader;
				delete vertexShader;
				delete program;
				emit result(nullptr, nullptr, nullptr, false, errors);
			}
			else
			{
				//wow. when we get there all's fine
				emit result(vertexShader, fragmentShader, program, true, QString());
			}
		}
	}
}

void GLSLCompileThread::run()
{
	//create context NOW
	/*if (!m_context->isValid())
	{
		m_context->create();
	}*/
	//loop while thread is running
	while (!m_quit)
	{
		m_mutex.lock();
		QString vertexCode = m_vertexCode;// .toLocal8Bit();
		QString fragmentCode = m_fragmentCode;// .toLocal8Bit();
		m_mutex.unlock();
		//check if we have valid data
		if (m_context && m_context->isValid() && !vertexCode.isEmpty() && !fragmentCode.isEmpty())
		{
			//make context current in this thread
			m_context->makeCurrent(m_surface);
			//now compile shader
			compileInternal(vertexCode, fragmentCode);
			//release context
			m_context->doneCurrent();
		}
		//wait until further notice
		m_mutex.lock();
		m_condition.wait(&m_mutex);
		m_mutex.unlock();
	}
}
