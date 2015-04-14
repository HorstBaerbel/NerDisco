#pragma once

#include <QWidget>
#include <QTextEdit>


class QTextEditStatusArea : public QWidget
{
    Q_OBJECT

public:
    QTextEditStatusArea(QTextEdit *editor);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QTextEdit * m_codeEditor;
};

