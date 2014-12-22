#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPaintEvent>
#include <QPainter>


class CodeEdit : public QTextEdit
{
    Q_OBJECT

public:
    struct Error
    {
        int line;
        int column;
        QString message;
    };

    explicit CodeEdit(QWidget * parent = 0);

    int getFirstVisibleBlockId();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void statusAreaPaintEvent(QPaintEvent *event);
    int statusAreaHeight();

    void setErrors(const QVector<Error> & errors);

protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *e);

private slots:
    void updateEditorMargins(int newBlockCount);
    void updateLineNumberArea(QRectF /*rect_f*/);
    void updateLineNumberArea(int /*slider_pos*/);
    void updateLineNumberArea();
    void updateHighlighting();

private:
    QWidget * m_lineNumberArea;
    QWidget * m_statusArea;
    QVector<Error> m_errors;
};
