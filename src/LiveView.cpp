#include "LiveView.h"

#include <QResizeEvent>
#include <QDebug>



const float LiveView::m_quadData[20] = {
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, 0.0f, 1.0f, 1.0f
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

const char * LiveView::m_frameBufferFragmentCode = "\
uniform sampler2D frameBufferTexture;\n\
\n\
varying vec2 texcoordVar;\n\
\n\
void main() {\n\
    gl_FragColor = texture2D(frameBufferTexture, texcoordVar);\n\
	//gl_FragColor = vec4(texcoordVar, 0.0, 1.0);\n\
}";


LiveView::LiveView(QWidget * parent)
	: QOpenGLWidget(parent)
	, m_vertexShader(nullptr)
	, m_fragmentShader(nullptr)
	, m_shaderProgram(nullptr)
	, m_fragmentScript(m_defaultFragmentCode)
	, m_scriptChanged(false)
//	, m_swapThread(nullptr)
	, m_compileThread(nullptr)
	, m_asynchronousCompilation(false)
	, m_grabFramebuffer(false)
	, m_mustInitialize(true)
	, m_renderRequested(false)
	, m_frameBufferVertexShader(nullptr)
	, m_frameBufferFragmentShader(nullptr)
	, m_frameBufferShaderProgram(nullptr)
	, m_frameBufferWidth(-1)
	, m_frameBufferHeight(-1)
	, m_keepAspect(false)
	, m_frameBufferObject(nullptr)
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
	delete m_frameBufferVertexShader;
	delete m_frameBufferFragmentShader;
	delete m_frameBufferShaderProgram;
	delete m_frameBufferObject;
	doneCurrent();
}

bool LiveView::hasHeightForWidth() const
{
	return true;
}

int LiveView::heightForWidth(int width) const
{
	return ((qreal)m_frameBufferHeight * (qreal)width / (qreal)m_frameBufferWidth);
}

QSize LiveView::sizeHint() const
{
	int w = m_frameBufferWidth;
	return QSize(w, heightForWidth(w));
}

void LiveView::setRenderSize(int width, int height)
{
	m_frameBufferWidth = width;
	m_frameBufferHeight = height;
	//setup orthographic projection matrix for vertex shader
	const float aspect = (float)width / (float)height;
	m_projectionMatrix.setToIdentity();
	if (width >= height)
	{
		m_projectionMatrix.ortho(-0.5f, 0.5f, -0.5f * aspect, 0.5f * aspect, 0.0f, 10.0f);
	}
	else
	{
		m_projectionMatrix.ortho(-0.5f, 0.5f, -0.5f / aspect, 0.5f / aspect, 0.0f, 10.0f);
	}
	//update widget geometry
	updateGeometry();
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

void LiveView::enableAsynchronousCompilation(bool enabled)
{
	m_asynchronousCompilation = enabled;
}

void LiveView::render()
{
	QMutexLocker locker(&m_grabMutex);
	m_renderRequested = true;
	update();
}

void LiveView::resizeGL(int width, int height)
{
	//check if the context is valid, else initialization will crash!
	if (context() && context()->isValid())
	{
		//setup viewport
		glViewport(0, 0, width, height);
	}
	//setup orthographic projection matrix for vertex shader
	m_projectionMatrix.setToIdentity();
	if (m_keepAspect)
	{
		//keep aspect by resizing the rendering
		const float aspect = (float)m_frameBufferWidth / (float)m_frameBufferHeight;
		if (m_frameBufferWidth >= m_frameBufferHeight)
		{
			m_projectionMatrix.ortho(-0.5f, 0.5f, -0.5f * aspect, 0.5f * aspect, 0.0f, 10.0f);
		}
		else
		{
			m_projectionMatrix.ortho(-0.5f, 0.5f, -0.5f / aspect, 0.5f / aspect, 0.0f, 10.0f);
		}
	}
	else
	{
		//draw full widget area
		m_projectionMatrix.ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f, 10.0f);
	}
}

void LiveView::initializeGL()
{
	//check if the context is valid, else initialization will crash!
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
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_RESCALE_NORMAL);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//set up framebuffer blit matrix
		m_blitMatrix.setToIdentity();
		m_blitMatrix.ortho(-0.5f, 0.5f, -0.5f, 0.5f, -1.0f, 1.0f);
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

void LiveView::CreateFrameBufferShader()
{
	if (!m_frameBufferShaderProgram)
	{
		QString errors;
		m_frameBufferFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
		if (!m_frameBufferFragmentShader->compileSourceCode(m_fragmentPrefix + m_frameBufferFragmentCode))
		{
			errors.append(m_frameBufferFragmentShader->log());
			delete m_frameBufferFragmentShader;
		}
		else
		{
			//fragment shader is ok. try compiling vertex shader
			m_frameBufferVertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
			if (!m_frameBufferVertexShader->compileSourceCode(m_vertexPrefix + m_defaultVertexCode))
			{
				errors.append(m_frameBufferVertexShader->log());
				delete m_frameBufferFragmentShader;
				delete m_frameBufferVertexShader;
			}
			else
			{
				//compilation worked. try linking
				m_frameBufferShaderProgram = new QOpenGLShaderProgram();
				m_frameBufferShaderProgram->addShader(m_frameBufferVertexShader);
				m_frameBufferShaderProgram->addShader(m_frameBufferFragmentShader);
				if (!m_frameBufferShaderProgram->link())
				{
					errors.append(m_frameBufferShaderProgram->log());
					delete m_frameBufferFragmentShader;
					delete m_frameBufferVertexShader;
					delete m_frameBufferShaderProgram;
				}
			}
		}
		if (!errors.isEmpty())
		{
			qDebug() << "Failed to create shader for blitting the framebuffer:" << errors;
		}
	}
}

void LiveView::CreateFrameBuffer()
{
	if (!m_frameBufferObject || m_frameBufferWidth == -1 || m_frameBufferHeight == -1)
	{
		m_frameBufferWidth = width();
		m_frameBufferHeight = height();
	}
	if (!m_frameBufferObject || m_frameBufferObject->width() != m_frameBufferWidth || m_frameBufferObject->height() != m_frameBufferHeight)
	{
		//destroy old framebuffer first
		if (m_frameBufferObject)
		{
			delete m_frameBufferObject;
		}
		//create new framebuffer
		QOpenGLFramebufferObjectFormat format;
		format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		//format.setMipmap(false);
		//format.setTextureTarget(GL_TEXTURE_2D);
		m_frameBufferObject = new QOpenGLFramebufferObject(m_frameBufferWidth, m_frameBufferHeight, format);
		qDebug() << "Framebuffer" << m_frameBufferObject->size();
	}
}

void LiveView::paintGL()
{
	//first lock mutex, so we can not grab the framebuffer or modify shaders at the same time
	QMutexLocker locker(&m_grabMutex);
	//make sure the widget is completely initialized and has been shown
	if (isValid())
	{
		//make context current
		makeCurrent();
		//allocate framebuffer frament shader and object
		CreateFrameBufferShader();
		CreateFrameBuffer();
		//allocate compile thread
		if (!m_compileThread)
		{
			//doneCurrent();
			m_compileThread = new GLSLCompileThread(context());
			connect(m_compileThread, SIGNAL(result(QOpenGLShader *, QOpenGLShader *, QOpenGLShaderProgram *, bool, const QString &)),
				this, SLOT(compilationFinished(QOpenGLShader *, QOpenGLShader *, QOpenGLShaderProgram *, bool, const QString &)));
			//initializes default vertex shader, default fragment shader and shader program
			m_scriptChanged = true;
			m_fragmentScript = m_defaultFragmentCode;
		}
		//if the script changed, compile it
		if (m_compileThread && m_scriptChanged)
		{
			if (m_asynchronousCompilation)
			{
				m_compileThread->compileAndLink(m_vertexPrefix + m_defaultVertexCode, m_fragmentPrefix + m_fragmentScript, m_asynchronousCompilation);
			}
			else
			{
				locker.unlock();
				m_compileThread->compileAndLink(m_vertexPrefix + m_defaultVertexCode, m_fragmentPrefix + m_fragmentScript, m_asynchronousCompilation);
				return;
			}
		}
		//check if we have a working shader
		if (m_frameBufferShaderProgram && m_frameBufferShaderProgram->isLinked() && m_shaderProgram && m_shaderProgram->isLinked())
		{
			//unbind default framebuffer and bind our framebuffer
			m_frameBufferObject->bind();
			//set up viewport for framebuffer size
			glViewport(0, 0, m_frameBufferWidth, m_frameBufferHeight);
			//clear view
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//now render script with shader
			m_shaderProgram->bind();
			//set all uniforms values
			m_shaderProgram->setUniformValue("projectionMatrix", m_projectionMatrix);
			m_shaderProgram->setUniformValue("renderSize", QVector2D(m_frameBufferWidth, m_frameBufferHeight));
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValues2d);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValues3d);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValues4d);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesf);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesui);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesi);
			setShaderUniformsFromMap(m_shaderProgram, m_shaderValuesb);
			//enable attributes in shader
			int position = m_shaderProgram->attributeLocation("position");
			int texcoord0 = m_shaderProgram->attributeLocation("texcoord0");
			glEnableVertexAttribArray(position); //position
			glEnableVertexAttribArray(texcoord0); //texture coordinates
			//setup vertex buffers
			glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), &m_quadData[0]);
			glVertexAttribPointer(texcoord0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), &m_quadData[3]);
			//render screen-sized quad
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			//disable attributes again
			glDisableVertexAttribArray(position);
			glDisableVertexAttribArray(texcoord0);
			//grab framebuffer now if needed
			if (m_grabFramebuffer)
			{
				m_grabbedFramebuffer = m_frameBufferObject->toImage();
				m_grabFramebuffer = false;
			}
			//undbind framebuffer and shader
			m_frameBufferObject->release();
			m_shaderProgram->release();
			//bind default widget framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
			//set viewport size to widget size
			glViewport(0, 0, width(), height());
			//now scale framebuffer to widget
			m_frameBufferShaderProgram->bind();
			//bind framebuffer texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_frameBufferObject->texture());
			//set all uniforms values
			m_frameBufferShaderProgram->setUniformValue("projectionMatrix", m_blitMatrix);
			m_frameBufferShaderProgram->setUniformValue("renderSize", QVector2D(width(), height()));
			m_frameBufferShaderProgram->setUniformValue("frameBufferTexture", 0);
			//enable attributes in shader
			position = m_frameBufferShaderProgram->attributeLocation("position");
			texcoord0 = m_frameBufferShaderProgram->attributeLocation("texcoord0");
			glEnableVertexAttribArray(position); //position
			glEnableVertexAttribArray(texcoord0); //texture coordinates
			//setup vertex buffers
			glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), &m_quadData[0]);
			glVertexAttribPointer(texcoord0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), &m_quadData[3]);
			//render screen-sized quad
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			//de-init everything again
			glDisableVertexAttribArray(position);
			glDisableVertexAttribArray(texcoord0);
			m_frameBufferShaderProgram->release();
			locker.unlock();
			//return and let the system do compositing etc. and send a frameSwapped signal
		}
		else
		{
			locker.unlock();
		}
/*		//tell the application we've rendered
		if (m_renderRequested)
		{
			m_renderRequested = false;
			emit renderingFinished();
		}*/
	}
}

/*void LiveView::resizeEvent(QResizeEvent * event)
{
	//QMutexLocker locker(&m_grabMutex);
	makeCurrent();
	QOpenGLWidget::resizeEvent(event);
	//resizeGL(m_frameBufferWidth, m_frameBufferHeight);
}*/

void LiveView::paintEvent(QPaintEvent * event)
{
	QMutexLocker locker(&m_grabMutex);
	if (m_renderRequested)
	{
		m_renderRequested = true;
		locker.unlock();
		paintGL();
	}
}

void LiveView::bufferSwapFinished()
{
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
	if (m_fragmentScript != script)
	{
		m_scriptChanged = true;
		m_fragmentScript = script;
	}
}

void LiveView::compilationFinished(QOpenGLShader * vertex, QOpenGLShader * fragment, QOpenGLShaderProgram * program, bool success, const QString & errors)
{
	QMutexLocker locker(&m_grabMutex);
	//the script has been recompiled. it does not matter if it worked or not, 
	//it does not need to be recompiled until setFragmentScript(const QString &) is called again...
	m_scriptChanged = false;
	if (success)
	{
		//make context current to free old shaders
		makeCurrent();
		delete m_shaderProgram;
		delete m_vertexShader;
		delete m_fragmentShader;
		//set new shaders
		m_vertexShader = vertex;
		m_fragmentShader = fragment;
		m_shaderProgram = program;
		locker.unlock();
		emit fragmentScriptChanged();
	}
	else
	{
		locker.unlock();
		emit fragmentScriptErrors(errors);
	}
}

QString LiveView::currentScriptPrefix() const
{
	return m_fragmentPrefix;
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
