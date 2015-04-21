#include "CodeEdit.h"
#include "QTextEditLineNumberArea.h"
#include "QTextEditStatusArea.h"
#include "ColorOperations.h"

#include <QColor>
#include <QScrollBar>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>


CodeEdit::CodeEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setTabStopWidth(4);
    setLineWrapMode(QTextEdit::NoWrap);
    setAcceptRichText(false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //add line numbers
    m_lineNumberArea = new QTextEditLineNumberArea(this);
    connect(this->document(), SIGNAL(blockCountChanged(int)), this, SLOT(updateEditorMargins(int)));
    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateLineNumberArea/*_2*/(int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(updateLineNumberArea()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateLineNumberArea()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateHighlighting()));
    updateEditorMargins(0);
    //add status area
    m_statusArea = new QTextEditStatusArea(this);
}

void CodeEdit::updateEditorMargins(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, statusAreaHeight());
}

//----- line numbers --------------------------------------------------

int CodeEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, this->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 13 +  fontMetrics().width(QLatin1Char('9')) * (digits);
    return space;
}

void CodeEdit::updateLineNumberArea(QRectF /*rect_f*/)
{
    CodeEdit::updateLineNumberArea();
}

void CodeEdit::updateLineNumberArea(int /*slider_pos*/)
{
    CodeEdit::updateLineNumberArea();
}

void CodeEdit::updateLineNumberArea()
{
    /* When the signal is emitted, the sliderPosition has been adjusted according to the action,
     * but the value has not yet been propagated (meaning the valueChanged() signal was not yet emitted),
     * and the visual display has not been updated. In slots connected to this signal you can thus safely
     * adjust any action by calling setSliderPosition() yourself, based on both the action and the
     * slider's value. */
    // Make sure the sliderPosition triggers one last time the valueChanged() signal with the actual value !!!!
    this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());
    // Since "QTextEdit" does not have an "updateRequest(...)" signal, we chose
    // to grab the imformations from "sliderPosition()" and "contentsRect()".
    // See the necessary connections used (Class constructor implementation part).
    QRect rect =  this->contentsRect();
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    updateEditorMargins(0);
    //----------
    int dy = this->verticalScrollBar()->sliderPosition();
    if (dy > -1) {
        m_lineNumberArea->scroll(0, dy);
    }
    // Addjust slider to alway see the number of the currently being edited line...
    int first_block_id = getFirstVisibleBlockId();
    if (first_block_id == 0 || this->textCursor().block().blockNumber() == first_block_id-1)
        this->verticalScrollBar()->setSliderPosition(dy-this->document()->documentMargin());
    //    // Snap to first line (TODO...)
    //    if (first_block_id > 0)
    //    {
    //        int slider_pos = this->verticalScrollBar()->sliderPosition();
    //        int prev_block_height = (int) this->document()->documentLayout()->blockBoundingRect(this->document()->findBlockByNumber(first_block_id-1)).height();
    //        if (dy <= this->document()->documentMargin() + prev_block_height)
    //            this->verticalScrollBar()->setSliderPosition(slider_pos - (this->document()->documentMargin() + prev_block_height));
    //    }
}

void CodeEdit::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);
    QRect cr = this->contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    m_statusArea->setGeometry(QRect(cr.left(), cr.bottom() - statusAreaHeight() + 1, cr.width(), statusAreaHeight()));
}

int CodeEdit::getFirstVisibleBlockId()
{
    // Detect the first block for which bounding rect - once translated
    // in absolute coordinated - is contained by the editor's text area
    // Costly way of doing but since "blockBoundingGeometry(...)" doesn't
    // exists for "QTextEdit"...

    QTextCursor curs = QTextCursor(this->document());
    curs.movePosition(QTextCursor::Start);
    for(int i=0; i < this->document()->blockCount(); ++i)
    {
        QTextBlock block = curs.block();
        QRect r1 = this->viewport()->geometry();
        QRect r2 = this->document()->documentLayout()->blockBoundingRect(block).translated(this->viewport()->geometry().x(), this->viewport()->geometry().y() - (this->verticalScrollBar()->sliderPosition()) ).toRect();
        if (r1.contains(r2, true)) { return i; }
        curs.movePosition(QTextCursor::NextBlock);
    }
    return 0;
}

void CodeEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());

    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    int blockNumber = this->getFirstVisibleBlockId();

    QTextBlock block = this->document()->findBlockByNumber(blockNumber);
    QTextBlock prev_block = (blockNumber > 0) ? this->document()->findBlockByNumber(blockNumber-1) : block;
    int translate_y = (blockNumber > 0) ? -this->verticalScrollBar()->sliderPosition() : 0;

    int top = this->viewport()->geometry().top();

    // Adjust text position according to the previous "non entirely visible" block
    // if applicable. Also takes in consideration the document's margin offset.
    int additional_margin;
    if (blockNumber == 0)
        // Simply adjust to document's margin
        additional_margin = (int) this->document()->documentMargin() -1 - this->verticalScrollBar()->sliderPosition();
    else
        // Getting the height of the visible part of the previous "non entirely visible" block
        additional_margin = (int) this->document()->documentLayout()->blockBoundingRect(prev_block).translated(0, translate_y).intersected(this->viewport()->geometry()).height();

    // Shift the starting point
    top += additional_margin;
    int bottom = top + (int) this->document()->documentLayout()->blockBoundingRect(block).height();

    QColor col_1 = QColor(Qt::blue).lighter(160);//(90, 255, 30);      // Current line (custom green)
    QColor col_0(120, 120, 120);    // Other lines  (custom darkgrey)

    // Draw the numbers (displaying the current line number in green)
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen((this->textCursor().blockNumber() == blockNumber) ? col_1 : col_0);
            painter.drawText(-5, top, m_lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) this->document()->documentLayout()->blockBoundingRect(block).height();
        ++blockNumber;
    }
}

//----- overpainting --------------------------------------------------

QColor mix(const QColor& c1, const QColor& c2, qreal t)
{
	if (t <= 0.0)
		return c1;
	if (t >= 1.0)
		return c2;
	if (std::isnan(t))
		return c1;
	qreal r = mix(c1.redF(), c2.redF(), t);
	qreal g = mix(c1.greenF(), c2.greenF(), t);
	qreal b = mix(c1.blueF(), c2.blueF(), t);
	qreal a = mix(c1.alphaF(), c2.alphaF(), t);
	return QColor::fromRgbF(r, g, b, a);
}

void CodeEdit::paintEvent(QPaintEvent *e)
{
    //call regulat paint event
    QTextEdit::paintEvent(e);
    //draw current line on top
/*    int blockNumber = textCursor().blockNumber();
    QTextBlock block = document()->findBlockByNumber(blockNumber);
    QTextBlock prev_block = (blockNumber > 0) ? document()->findBlockByNumber(blockNumber-1) : block;
    int translate_y = (blockNumber > 0) ? -verticalScrollBar()->sliderPosition() : 0;
    int top = this->viewport()->geometry().top();
    // Adjust text position according to the previous "non entirely visible" block
    // if applicable. Also takes in consideration the document's margin offset.
    int additional_margin;
    if (blockNumber == 0)
        // Simply adjust to document's margin
        additional_margin = (int) document()->documentMargin() -1 - verticalScrollBar()->sliderPosition();
    else
        // Getting the height of the visible part of the previous "non entirely visible" block
        additional_margin = (int) document()->documentLayout()->blockBoundingRect(prev_block).translated(0, translate_y).intersected(viewport()->geometry()).height();
    // Shift the starting point
    top += additional_margin;
    int bottom = top + (int) document()->documentLayout()->blockBoundingRect(block).height();
    QPainter painter(viewport());
    painter.setOpacity(0.5);
    qreal blockCoords[4];
    document()->documentLayout()->blockBoundingRect(block).getRect(&blockCoords[0], &blockCoords[1], &blockCoords[2], &blockCoords[3]);
    painter.fillRect(QRectF(0, blockCoords[1], viewport()->width(), blockCoords[3]), Qt::blue);*/
}

void CodeEdit::updateHighlighting()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        //set up colors
        QColor errorColor(QColor(Qt::red).lighter(150));
        QColor cursorColor(QColor(Qt::blue).lighter(180));
        QColor mixColor(mix(errorColor, cursorColor, 0.5));
        //highlight errors first
        bool cursorDone = false;
        foreach (const Error & error, m_errors)
        {
            QTextBlock block = document()->findBlockByLineNumber(error.line - 1); //subtract 1, because line numbers start at 1
            QTextCursor cursor(document());
            cursor.setPosition(block.position(), QTextCursor::MoveAnchor);
            cursor.setPosition(block.position(), QTextCursor::KeepAnchor);
            QTextEdit::ExtraSelection selection;
            selection.cursor = cursor;
            selection.cursor.clearSelection();
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            if (textCursor().blockNumber() == cursor.blockNumber())
            {
                selection.format.setBackground(mixColor);
                cursorDone = true;
            }
            else
            {
                selection.format.setBackground(errorColor);
            }
            extraSelections.append(selection);
        }
        //check if cursor is already done
        if (!cursorDone)
        {
            QTextEdit::ExtraSelection selection;
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.format.setBackground(cursorColor);
            extraSelections.append(selection);
        }
    }
    setExtraSelections(extraSelections);
    //now update the status area too to show error messages
    m_statusArea->update();
}

//----- status area --------------------------------------------------

void CodeEdit::statusAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_statusArea);
    //fill background
    painter.fillRect(event->rect(), Qt::lightGray);
    //draw errors message
    if (m_errors.size() > 0)
    {
        bool errorUnderCursor = false;
        //check if a line is selected in which an error occured
        foreach (Error error, m_errors)
        {
            if (textCursor().block().position() == document()->findBlockByLineNumber(error.line - 1).position())
            {
                errorUnderCursor = true;
                //show error description for line
                painter.setPen(QColor(Qt::red).lighter(120));
                painter.drawText(event->rect().adjusted(lineNumberAreaWidth() + 2, 2, 0, 0), QString("Error in line %2: %1").arg(error.message).arg(error.line));
            }
        }
        if (!errorUnderCursor)
        {
             //if no line with an error is selected, show general error message
            painter.setPen(QColor(Qt::red).lighter(120));
            painter.drawText(event->rect().adjusted(lineNumberAreaWidth() + 2, 2, 0, 0), QString("Script errors (place cursor over line to see description)"));
        }
    }
    else
    {
        //script is ok. show compile success
        painter.setPen(QColor(Qt::green).darker(150));
        painter.drawText(event->rect().adjusted(lineNumberAreaWidth() + 2, 2, 0, 0), QString("Script ok"));
    }
}

int CodeEdit::statusAreaHeight()
{
    return (fontMetrics().height() + 6);
}

//----- errors --------------------------------------------------

void CodeEdit::setErrors(const QVector<Error> & errors)
{
    m_errors = errors;
    updateHighlighting();
	if (errors.isEmpty())
	{
		setToolTip("");
	}
	else
	{
		setToolTip(errors.at(0).message);
	}
}
