#pragma once

#include "Parameters.h"

#include <QObject>
#include <QImage>
#include <QDomDocument>


class DisplayImageConverter : public QObject
{
	Q_OBJECT

public:
	DisplayImageConverter(QObject * parent = NULL);

	/// @brief Save the current settings to an XML document.
	/// @param parent The paren element to write the settings to.
	void toXML(QDomElement & parent) const;
	/// @brief Read current settings from XML document.
	/// @param parent The parent element to load the settings from.
	DisplayImageConverter & fromXML(const QDomElement & parent);

	ParameterInt displayWidth;
	ParameterInt displayHeight;
	ParameterFloat crossfadeValue;
	ParameterFloat displayGamma;
	ParameterFloat displayBrightness;
	ParameterFloat displayContrast;

	void convertImages(const QImage & a, const QImage & b);

signals:
	void previewImageChanged(const QImage & image);
	void displayImageChanged(const QImage & image);

private:
	QImage m_previewImage;
	QImage m_displayImage;
};
