#pragma once

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QOpenGLWidget>


class SwapThread : public QThread
{
	Q_OBJECT

public:
	SwapThread(QObject * parent);
	~SwapThread();

	///@brief Do an asynchronous buffer swap.
	///@note ONLY CALL THIS FROM THE MAIN THREAD!
	/// The object will emit bufferSwapFinished() when the buffer swap is done.
	void swapBuffersAsync(QOpenGLWidget * widget);

protected:
	void run();

signals:
	/// @brief swapBuffersAsync() was called and the asynchronous SwapBuffers() call has finished.
	void bufferSwapFinished();

private:
	QOpenGLWidget * m_widget;
	QMutex m_mutex;
	QWaitCondition m_condition;
	bool m_quit;
	QAtomicInt m_isSwapping;
};
