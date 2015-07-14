#include "QAspectRatioLabel.h"


QAspectRatioLabel::QAspectRatioLabel(QWidget * parent)
	: QLabel(parent)
	, m_transformationMode(Qt::SmoothTransformation)
{
	//setMinimumSize(1, 1);
}

int QAspectRatioLabel::heightForWidth(int width) const
{
	return ((qreal)m_pixmap.height() * (qreal)width / (qreal)m_pixmap.width());
}

QSize QAspectRatioLabel::sizeHint() const
{
	const int w = width();
	return QSize(w, heightForWidth(w));
}

void QAspectRatioLabel::setPixmap(const QPixmap & pixmap)
{
	m_pixmap = pixmap;
	QLabel::setPixmap(pixmap);
	updateGeometry();
}

void QAspectRatioLabel::setTransformationMode(Qt::TransformationMode mode)
{
	m_transformationMode = mode;
}

void QAspectRatioLabel::resizeEvent(QResizeEvent * event)
{
	QLabel::setPixmap(m_pixmap.scaled(size(), Qt::KeepAspectRatio, m_transformationMode));
}
