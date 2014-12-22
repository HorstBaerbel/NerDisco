#include "liveview.h"
#include "settings.h"

#include <QDebug>
#include <QQmlEngine>


LiveView::LiveView(QWidget * parent)
    : QWidget(parent)
    , m_quickView(new QQuickView())
    , m_frameworkItem(NULL)
    , m_renderScriptItem(NULL)
{
    Settings & settings = Settings::getInstance();
    setFixedSize(settings.frameBufferWidth(), settings.frameBufferHeight());
    //create a new layout and add the view
    QHBoxLayout * dummyLayout = new QHBoxLayout();
    dummyLayout->setMargin(0);
    dummyLayout->setSpacing(0);
    setLayout(dummyLayout);
    dummyLayout->addWidget(QWidget::createWindowContainer(m_quickView));//, 0, Qt::AlignCenter);
    //create the component holding the render script
    m_quickView->setSource(QUrl("qrc:/framework.qml"));
    //set up item size
    m_quickView->rootObject()->setProperty("width", settings.frameBufferWidth());
    m_quickView->rootObject()->setProperty("height", settings.frameBufferHeight());
    //retrieve the framework qml item
    m_frameworkItem = m_quickView->rootObject()->findChild<QQuickItem*>("framework");
    //when the script inside the framework changes we get notified
    connect(m_frameworkItem, SIGNAL(renderScriptChanged()), this, SLOT(updateRenderScriptItem()), Qt::QueuedConnection);
}

void LiveView::setRenderScript(const QString & script)
{
    if (m_frameworkItem != NULL)
    {
		//clear the render item. it will probably be new...
		m_renderScriptItem = NULL;
        //set the new program string to the framework. it will be compiles internally
        m_frameworkItem->setProperty("renderScriptText", script.toUtf8());
    }
}

void LiveView::updateRenderScriptItem()
{
    if (m_frameworkItem != NULL)
    {
        //try to find the new render item
        m_renderScriptItem = m_frameworkItem->findChild<QQuickItem*>("renderScript");
        if (m_renderScriptItem != NULL)
        {
            emit renderScriptChanged();
        }
    }
}

void LiveView::setRenderScriptProperty(const QString & name, const QVariant & value)
{
    if (m_renderScriptItem != NULL)
    {
        m_renderScriptItem->setProperty(name.toUtf8().constData(), value);
    }
}

QImage LiveView::grabFramebuffer()
{
    return m_quickView->grabWindow();
}

QQuickView * LiveView::quickView()
{
    return m_quickView;
}
