#include "ImageOperations.h"


QImage & changeImage(QImage & image, float brightness, float contrast, float gamma)
{
	switch (image.format())
	{
		case QImage::Format_RGB32:
		case QImage::Format_ARGB32:
		case QImage::Format_ARGB32_Premultiplied:
			for (int y = 0; y < image.height(); ++y)
			{
				unsigned char * scanLine = image.scanLine(y);
				for (int x = 0; x < image.width() * 4; x += 4)
				{
					scanLine[x + 1] = change(scanLine[x + 1], brightness, contrast, gamma);
					scanLine[x + 2] = change(scanLine[x + 2], brightness, contrast, gamma);
					scanLine[x + 3] = change(scanLine[x + 3], brightness, contrast, gamma);
				}
			}
			break;
		case QImage::Format_RGB888:
			for (int y = 0; y < image.height(); ++y)
			{
				unsigned char * scanLine = image.scanLine(y);
				for (int x = 0; x < image.width() * 3; ++x)
				{
					scanLine[x] = change(scanLine[x], brightness, contrast, gamma);
				}
			}
			break;
		case QImage::Format_RGBX8888:
		case QImage::Format_RGBA8888:
		case QImage::Format_RGBA8888_Premultiplied:
			for (int y = 0; y < image.height(); ++y)
			{
				unsigned char * scanLine = image.scanLine(y);
				for (int x = 0; x < image.width() * 4; x += 4)
				{
					scanLine[x] = change(scanLine[x], brightness, contrast, gamma);
					scanLine[x + 1] = change(scanLine[x + 1], brightness, contrast, gamma);
					scanLine[x + 2] = change(scanLine[x + 2], brightness, contrast, gamma);
				}
			}
			break;
	}
	return image;
}