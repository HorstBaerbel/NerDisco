#pragma once

#include <QLabel>


class QAspectRatioLabel : public QLabel
{
	Q_OBJECT

public:
	explicit QAspectRatioLabel(QWidget * parent = nullptr);
	virtual int heightForWidth(int width) const override;
	virtual QSize sizeHint() const;

public slots:
	void setPixmap(const QPixmap & pixmap);
	void setTransformationMode(Qt::TransformationMode mode = Qt::SmoothTransformation);

protected:
	void resizeEvent(QResizeEvent * event);

private:
	QPixmap m_pixmap;
	Qt::TransformationMode m_transformationMode;
};
