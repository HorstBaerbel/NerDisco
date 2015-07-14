#pragma once

#include <QOpenGLWindow>
#include <QOpenGLFunctions>


class OutputWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	OutputWindow(QScreen * screen);

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

private slots:
	void resolutionChanged(int width, int height);

};
