#include "LiveView.h"

#include <QResizeEvent>
#include <QDebug>



const float LiveView::m_quadData[20] = {
	-1.0f, -1.0f, 0.0, 0.0f, 0.0f,
	-1.0f,  1.0f, 0.0, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0, 1.0f, 0.0f,
	 1.0f,  1.0f, 0.0, 1.0f, 1.0f
};

const char * LiveView::m_vertexPrefixGLES2 = "\
#version 100\n\
\n";

const char * LiveView::m_fragmentPrefixGLES2 = "\
#version 100\n\
precision highp float;\n\
\n";

const char * LiveView::m_vertexPrefixGL2 = "\
#version 120\n\
\n";

const char * LiveView::m_fragmentPrefixGL2 = "\
#version 120\n\
\n";

const char * LiveView::m_defaultVertexCode = "\
uniform mat4 projectionMatrix;\n\
\n\
attribute vec3 position;\n\
attribute vec2 texcoord0;\n\
\n\
varying vec2 texcoordVar;\n\
\n\
void main() {\n\
    gl_Position = projectionMatrix * vec4(position, 1.0);\n\
    texcoordVar = texcoord0;\n\
}";

const char * LiveView::m_defaultFragmentCode = "\
uniform vec2 renderSize;\n\
\n\
varying vec2 texcoordVar;\n\
\n\
void main() {\n\
    gl_FragColor = vec4(texcoordVar, 0.0, 1.0);\n\
}";


LiveView::LiveView(QWidget * parent)
	: QOpenGLWidget(parent)
	, m_vertexShader(NULL)
	, m_fragmentShader(NULL)
	, m_shaderProgram(NULL)
	, m_fragmentScript(m_defaultFragmentCode)
	, m_scriptChanged(true)
//	, m_swapThread(NULL)
	, m_grabFramebuffer(false)
	, m_mustInitialize(true)
	, m_renderRequested(false)
{
	//create buffer swapping thread
	//m_swapThread = new SwapThread(this);
	//connect(m_swapThread, SIGNAL(bufferSwapFinished()), this, SLOT(bufferSwapFinished()), Qt::QueuedConnection);
	connect(this, SIGNAL(frameSwapped()), this, SLOT(bufferSwapFinished()));
	//make sure the widget is not grabbing the context
}

LiveView::~LiveView()
{
	//make context current so resources can be released
	makeCurrent();
	delete m_vertexShader;
	delete m_fragmentShader;
	delete m_shaderProgram;
	doneCurrent();
}

QSurfaceFormat LiveView::getDefaultFormat()
{
	QSurfaceFormat format(QSurfaceFormat::defaultFormat());
	//format.setProfile(QSurfaceFormat::CompatibilityProfile);
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	format.setVersion(2, 1);
/*	format.setRedBufferSize(8);
	format.setGreenBufferSize(8);
	format.setBlueBufferSize(8);
	format.setAlphaBufferSize(8);
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);*/
	return format;
}

void LiveView::render()
{
	QMutexLocker locker(&m_grabMutex);
	m_renderRequested = true;
	update();
}

void LiveView::resizeGL(int width, int height)
{
	if (m_mustInitialize)
	{
		initializeGL();
	}
	//try to make the context current
	makeCurrent();
	//check if the context is valid, else inuitialization will crash!
	if (context() && context()->isValid())
	{
		//setup viewport
		glViewport(0, 0, width, height);
	}
}

void LiveView::initializeGL()
{
	//try to make the context current
	makeCurrent();
	//check if the context is valid, else inuitialization will crash!
	if (context() && context()->isValid())
	{
		//check which OpenGL backend we're using and switch shader prefixes accordingly
		m_vertexPrefix = context()->isOpenGLES() ? m_vertexPrefixGLES2 : m_vertexPrefixGL2;
		m_fragmentPrefix = context()->isOpenGLES() ? m_fragmentPrefixGLES2 : m_fragmentPrefixGL2;
		//initialize opengl function bindings
		initializeOpenGLFunctions();
		//setup some OpenGL stuff
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		//glEnable(GL_TEXTURE_2D);
		glEnable(GL_RESCALE_NORMAL);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//setup orthographic projection matrix for vertex shader
		m_projectionMatrix.ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
		m_mustInitialize = false;
	}
}

template <class T>
void setShaderUniformsFromMap(QOpenGLShaderProgram * shaderProgram, const T & t)
{
    typename T::const_iterator iter = t.cbegin();
	while (iter != t.cend())
	{
		shaderProgram->setUniformValue(iter.key().toLocal8Bit().constData(), iter.value());
		++iter;
	}
}

void LiveView::paintGL()
{
	//first lock mutex, so we can not grab the framebuffer or modify shaders at the same time
	QMutexLocker locker(&m_grabMutex);
	//check if we need to initialize the widget
	if (m_mustInitialize)
	{
		initializeGL();
	}
	//make sure the widget is completely initialized and has been shown
	if (isValid())
	{
		//make context current
		makeCurrent();
		//check if the render script changed and we need to recompile
		if (m_scriptChanged)
		{
			QString errors;
			//initializes default vertex shader, empty fragment shader and shader program
			if (m_shaderProgram == NULL && m_vertexShader == NULL && m_fragmentShader == NULL)
			{
				//compile default vertex shader
				m_vertexShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
				if (!m_vertexShader->compileSourceCode(m_vertexPrefix + m_defaultVertexCode))
				{
					errors.append(m_vertexShader->log());
				}
				//compile default fragment shader
				m_fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
				if (!m_fragmentShader->compileSourceCode(m_fragmentPrefix + m_defaultFragmentCode))
				{
					errors.append(m_fragmentShader->log());
				}
				//add both shaders to program
				m_shaderProgram = new QOpenGLShaderProgram(this);
				m_shaderProgram->addShader(m_vertexShader);
				m_shaderProgram->addShader(m_fragmentShader);
				//try linking the program together
				if (!m_shaderProgram->link())
				{
					errors.append(m_shaderProgram->log());
				}
			}
			//if the fragment script changed, update and compile it and re-link the shader
			QOpenGLShader * newShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
			//try to compile new fragment shader code
			if (!newShader->compileSourceCode(m_fragmentPrefix + m_fragmentScript))
			{
				errors.append(newShader->log());
			}
			else
			{
				//compilation succeeded. remove old fragment shader from program
				m_shaderProgram->removeShader(m_fragmentShader);
				//add fragment shader to program and try re-linking
				m_shaderProgram->addShader(newShader);
				if (m_shaderProgram->link())
				{
					//re-linking worked. replace fragment code and shader
					delete m_fragmentShader;
					m_fragmentShader = newShader;
					locker.unlock();
					emit fragmentScriptChanged();
				}
				else
				{
					//linking failed. use old shader again. this should work
					errors.append(m_shaderProgram->log());
					m_shaderProgram->removeShader(newShader);
					m_shaderProgram->addShader(m_fragmentShader);
					delete newShader;
					m_shaderProgram->link();
				}
			}
			//mark the script as being NOT changed. we have tried to compile and won't do so again before another change
			m_scriptChanged = false;
			//if errors occurred, notify the application
			if (!errors.isEmpty())
			{
				locker.unlock();
				emit fragmentScriptErrors(errors);
			}
		}
		//check if we have a working shader
		if (m_shaderProgram && m_shaderProgram->isLinked())
		{
			//clear view
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//now render with shader
			m_shaderProgram->bind();
			m_shaderProgram->setUniformValue("projectionMatrix", m_projectionMatrix);
			m_shaderProgram->setUniformValue("renderSize", QVector2D(width(), height()));
			//set all uniforms stored in the maps
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValues2d);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValues3d);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValues4d);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesf);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesui);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesi);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesb);
			//setup vertex buffers
			const int position = m_shaderProgram->attributeLocation("position");
			const int texcoord0 = m_shaderProgram->attributeLocation("texcoord0");
			glEnableVertexAttribArray(position); //position
			glEnableVertexAttribArray(texcoord0); //texture coordinates
			glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), &m_quadData[0]);
			glVertexAttribPointer(texcoord0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), &m_quadData[3]);
			//render screen-sized quad
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			//de-init everything again
			glDisableVertexAttribArray(position);
			glDisableVertexAttribArray(texcoord0);
			m_shaderProgram->release();
			locker.unlock();
			//return and let the system do compositing etc. and send a frameSwapped signal
		}
		else
		{
			locker.unlock();
			emit renderingFinished();
		}
	}
}

void LiveView::resizeEvent(QResizeEvent * event)
{
	QMutexLocker locker(&m_grabMutex);
	makeCurrent();
	resizeGL(event->size().width(), event->size().height());
}

void LiveView::paintEvent(QPaintEvent * event)
{
	QMutexLocker locker(&m_grabMutex);
	if (m_renderRequested)
	{
		m_renderRequested = false;
		locker.unlock();
		paintGL();
	}
}

void LiveView::bufferSwapFinished()
{
	m_grabMutex.lock();
	if (m_grabFramebuffer && isValid())
	{
		m_grabFramebuffer = false;
		m_grabMutex.unlock();
		//grab framebuffer. to our surprise this calls paintGL AGAIN.
		m_grabbedFramebuffer = grabFramebuffer();
	}
	else
	{
		m_grabMutex.unlock();
	}
	emit renderingFinished(); 
}

void LiveView::grabFramebufferAfterSwap()
{
	QMutexLocker locker(&m_grabMutex);
	m_grabFramebuffer = true;
}

QImage LiveView::getGrabbedFramebuffer()
{
	QMutexLocker locker(&m_grabMutex);
	return m_grabbedFramebuffer;
}

void LiveView::setFragmentScript(const QString & script)
{
	QMutexLocker locker(&m_grabMutex);
	m_scriptChanged = true;
	m_fragmentScript = script;
}

void LiveView::setFragmentScriptProperty(const QString & name, const QVector2D & value)
{
	m_shaderValues2d[name] = value;
}

void LiveView::setFragmentScriptProperty(const QString & name, const QVector3D & value)
{
	m_shaderValues3d[name] = value;
}

void LiveView::setFragmentScriptProperty(const QString & name, const QVector4D & value)
{
	m_shaderValues4d[name] = value;
}

void LiveView::setFragmentScriptProperty(const QString & name, float value)
{
	m_shaderValuesf[name] = value;
}

void LiveView::setFragmentScriptProperty(const QString & name, double value)
{
	m_shaderValuesf[name] = (float)value;
}

void LiveView::setFragmentScriptProperty(const QString & name, unsigned int value)
{
	m_shaderValuesui[name] = value;
}

void LiveView::setFragmentScriptProperty(const QString & name, int value)
{
	m_shaderValuesi[name] = value;
}

void LiveView::setFragmentScriptProperty(const QString & name, bool value)
{
	m_shaderValuesb[name] = value;
}
