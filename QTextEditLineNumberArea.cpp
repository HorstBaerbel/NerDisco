#include "QTextEditLineNumberArea.h"
#include "CodeEdit.h"


QTextEditLineNumberArea::QTextEditLineNumberArea(QTextEdit *editor)
    : QWidget(editor)
    , m_codeEditor(editor)
{
}

QSize QTextEditLineNumberArea::sizeHint() const
{
    return QSize(((CodeEdit*)m_codeEditor)->lineNumberAreaWidth(), 0);
}

void QTextEditLineNumberArea::paintEvent(QPaintEvent *event)
{
    ((CodeEdit *)m_codeEditor)->lineNumberAreaPaintEvent(event);
}
