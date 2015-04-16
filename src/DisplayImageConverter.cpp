#include "DisplayImageConverter.h"

#include "ImageOperations.h"
#include <QPainter>


DisplayImageConverter::DisplayImageConverter(QObject * parent)
	: QObject(parent)
	, displayWidth("displayWidth", 32, 8, 64)
	, displayHeight("displayHeight", 18, 4, 64)
	, displayBrightness("displayBrightness", 0, -50, 50)
	, displayContrast("displayContrast", 0, -50, 50)
	, displayGamma("displayGamma", 220, 100, 400)
	, crossFadeValue("crossFadeValue", 0, 0, 100)
{
}

void DisplayImageConverter::toXML(QDomElement & parent) const
{
	//try to find element in parent
	QDomElement element = parent.firstChildElement("DisplayImageConverter");
	if (element.isNull())
	{
		//add the new element
		element = parent.ownerDocument().createElement("DisplayImageConverter");
		parent.appendChild(element);
	}
	displayWidth.toXML(element);
	displayHeight.toXML(element);
	crossFadeValue.toXML(element);
	displayBrightness.toXML(element);
	displayContrast.toXML(element);
	displayGamma.toXML(element);
}

DisplayImageConverter& DisplayImageConverter::fromXML(const QDomElement & parent)
{
	//try to find element in document
	QDomElement element = parent.firstChildElement("DisplayImageConverter");
	if (element.isNull())
	{
		throw std::runtime_error("No audio device settings found!");
	}
	displayWidth.fromXML(element);
	displayHeight.fromXML(element);
	crossFadeValue.fromXML(element);
	displayBrightness.fromXML(element);
	displayContrast.fromXML(element);
	displayGamma.fromXML(element);
	return *this;
}

void DisplayImageConverter::convertImages(const QImage & a, const QImage & b)
{
	//allocate image if it isn't
	if (m_previewImage.isNull() || m_previewImage.size() != a.size())
	{
		m_previewImage = QImage(a.size(), a.format());
	}
	//composite images
	QPainter painter(&m_previewImage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(m_previewImage.rect(), Qt::black);
	qreal alphaB = crossFadeValue.normalizedValue();
	qreal alphaA = 1.0 - alphaB;
	if (alphaA <= alphaB)
	{
		painter.drawImage(0, 0, a);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setOpacity(alphaB);
		painter.drawImage(0, 0, b);
	}
	else
	{
		painter.drawImage(0, 0, b);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setOpacity(alphaA);
		painter.drawImage(0, 0, a);
	}
	//scale image down to real size
	m_displayImage = m_previewImage.scaled(displayWidth, displayHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	//do image correction
	float brightness = displayBrightness / 50.0f;
	float contrast = (displayContrast + 50.0f) / 100.0f * 2.0f;
	float gamma = displayGamma / 220.0f;
	m_displayImage = changeImage(m_displayImage, brightness, contrast, gamma);
	//send results
	displayImageChanged(m_displayImage);
	previewImageChanged(m_previewImage);
}
