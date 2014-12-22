#include "ColorOperations.h"


QColor mix(const QColor& c1, const QColor& c2, qreal bias)
{
    if (bias <= 0.0)
        return c1;
    if (bias >= 1.0)
        return c2;
    if (std::isnan(bias))
        return c1;
    qreal r = mix(c1.redF(), c2.redF(), bias);
    qreal g = mix(c1.greenF(), c2.greenF(), bias);
    qreal b = mix(c1.blueF(), c2.blueF(), bias);
    qreal a = mix(c1.alphaF(), c2.alphaF(), bias);
    return QColor::fromRgbF(r, g, b, a);
}
