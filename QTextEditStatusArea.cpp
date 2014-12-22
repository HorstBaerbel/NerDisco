#include "QTextEditStatusArea.h"
#include "CodeEdit.h"


QTextEditStatusArea::QTextEditStatusArea(QTextEdit *editor)
    : QWidget(editor)
    , m_codeEditor(editor)
{
}

QSize QTextEditStatusArea::sizeHint() const
{
    return QSize(0, ((CodeEdit*)m_codeEditor)->statusAreaHeight());
}

void QTextEditStatusArea::paintEvent(QPaintEvent *event)
{
    ((CodeEdit *)m_codeEditor)->statusAreaPaintEvent(event);
}

