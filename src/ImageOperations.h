#pragma once

#include "ColorOperations.h"
#include <QImage>


/// @brief Change image brightness, contrast and gamma value.
/// @param image Image to change.
/// @param brightness Brightness offset to add to image.
/// @param contrast Factor to multiply image by.
/// @param gamma Gamma factor to apply to image.
/// @return Updated image.
QImage & changeImage(QImage & image, float brightness, float contrast, float gamma);
