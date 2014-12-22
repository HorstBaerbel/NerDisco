#pragma once

#include <QQuickItem>
#include <QQuickView>
#include <QQmlComponent>
#include <QTime>
#include <QWidget>
#include <QHBoxLayout>


class LiveView : public QWidget
{
    Q_OBJECT

public:
    LiveView(QWidget * parent = 0);

    void setRenderScript(const QString & script);
    void setRenderScriptProperty(const QString & name, const QVariant & value);
    QImage grabFramebuffer();

    QQuickView * quickView();

signals:
    void renderScriptChanged();

protected slots:
    void updateRenderScriptItem();

private:
    QQuickView * m_quickView;
    QQuickItem * m_frameworkItem;
    QQuickItem * m_renderScriptItem;
};

