#pragma once

#include <QWidget>
#include <QTextEdit>


class QTextEditLineNumberArea : public QWidget
{
    Q_OBJECT

public:
    QTextEditLineNumberArea(QTextEdit *editor);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QTextEdit * m_codeEditor;
};
