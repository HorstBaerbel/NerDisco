#include "OutputWindow.h"


OutputWindow::OutputWindow(QScreen * screen)
{
}

void OutputWindow::initializeGL()
{
	//check if the context is valid, else initialization will crash!
	if (context() && context()->isValid())
	{
		//check which OpenGL backend we're using and switch shader prefixes accordingly
		//m_vertexPrefix = context()->isOpenGLES() ? m_vertexPrefixGLES2 : m_vertexPrefixGL2;
		//m_fragmentPrefix = context()->isOpenGLES() ? m_fragmentPrefixGLES2 : m_fragmentPrefixGL2;
		//setup some OpenGL stuff
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		//glEnable(GL_TEXTURE_2D);
		glEnable(GL_RESCALE_NORMAL);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//setup orthographic projection matrix for vertex shader
		//m_projectionMatrix.ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	}
}

void OutputWindow::resizeGL(int width, int height)
{
	//check if the context is valid, else initialization will crash!
	if (context() && context()->isValid())
	{
		//setup viewport
		glViewport(0, 0, width, height);
	}
}

void OutputWindow::paintGL()
{

}

void OutputWindow::resolutionChanged(int width, int height)
{
}