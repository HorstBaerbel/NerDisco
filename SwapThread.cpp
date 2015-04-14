#include "SwapThread.h"

#include <QApplication>
#include <QDebug>


SwapThread::SwapThread(QObject * parent)
	: QThread(parent)
	, m_context(NULL)
	, m_quit(false)
	, m_isSwapping(false)
{
}

SwapThread::~SwapThread()
{
	m_mutex.lock();
	m_quit = true;
	m_context = NULL;
	m_condition.wakeOne();
	m_mutex.unlock();
	wait();
}

void SwapThread::swapBuffersAsync(QGLContext * context)
{
	QMutexLocker locker(&m_mutex);
	//unbind context from main thread
	m_context = context;
	m_context->doneCurrent();
	//move object to our thread. this can only be done in the thread you're moving from...
	m_context->moveToThread(this);
	//wake up thread or start if it isn't running
	if (!isRunning())
		start();
	else
		m_condition.wakeOne();
}

void SwapThread::run()
{
	QGLContext * context = NULL;

	while (!m_quit) {
		//when we ge here the main thread must have called doneCurrent() for m_context
		m_mutex.lock();
		context = m_context;
		m_context = NULL;
		//we only do something if a context was set in swapBuffersAsync()
		if (context)
		{
			//mark swapping as in progress
			m_isSwapping.fetchAndStoreAcquire(true);
			//make context current in this thread
			context->makeCurrent();
			//do blocking buffer swap
			context->swapBuffers();
			//unbind context from this thread
			context->doneCurrent();
			//move context back to main thread
			context->moveToThread(qApp->thread());
			//clear context variable again
			context = NULL;
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
