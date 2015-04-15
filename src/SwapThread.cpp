#include "SwapThread.h"

#include <QApplication>
#include <QOpenGLContext>
#include <QDebug>


SwapThread::SwapThread(QObject * parent)
	: QThread(parent)
	, m_widget(NULL)
	, m_quit(false)
	, m_isSwapping(false)
{
}

SwapThread::~SwapThread()
{
	m_mutex.lock();
	m_quit = true;
	m_widget = NULL;
	m_condition.wakeOne();
	m_mutex.unlock();
	wait();
}

void SwapThread::swapBuffersAsync(QOpenGLWidget * widget)
{
	QMutexLocker locker(&m_mutex);
	//unbind context from main thread
	m_widget = widget;
	m_widget->doneCurrent();
	//move object to our thread. this can only be done in the thread you're moving from...
	m_widget->context()->moveToThread(this);
	//wake up thread or start if it isn't running
	if (!isRunning())
		start();
	else
		m_condition.wakeOne();
}

void SwapThread::run()
{
	QOpenGLWidget * widget = NULL;

	while (!m_quit) {
		//when we ge here the main thread must have called doneCurrent() for m_context
		m_mutex.lock();
		widget = m_widget;
		m_widget = NULL;
		//we only do something if a context was set in swapBuffersAsync()
		if (widget)
		{
			//mark swapping as in progress
			m_isSwapping.fetchAndStoreAcquire(true);
			//make context current in this thread
			widget->makeCurrent();
			//do blocking buffer swap
			//widget->context()->swapBuffers(widget);
			//unbind context from this thread
			widget->doneCurrent();
			//move context back to main thread
			widget->context()->moveToThread(qApp->thread());
			//clear context variable again
			widget = NULL;
			//mark swapping as finished again
			m_isSwapping.fetchAndStoreAcquire(false);
			//signal that we're done swapping
			emit bufferSwapFinished();
		}
		m_mutex.unlock();
		//wait for next buffer swap
		m_mutex.lock();
		m_condition.wait(&m_mutex);
		if (m_quit)
		{
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
	}
}
