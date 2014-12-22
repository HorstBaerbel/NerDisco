#pragma once

#include <cmath>
#include <QColor>


template<class T>
inline const T& kClamp( const T& x, const T& low, const T& high )
{
    if ( x < low ) return low;
    else if ( high < x ) return high;
    else return x;
}

inline int changeBrightness(int value, float brightness)
{
    return kClamp(value + brightness * 255.0f, 0.0f, 255.0f);
}

inline int changeContrast(int value, float contrast)
{
    return kClamp((( value - 127.0f ) * contrast ) + 127.0f, 0.0f, 255.0f);
}

inline int changeGamma(int value, float gamma)
{
    return kClamp(std::pow( value / 255.0f, gamma ) * 255.0f, 0.0f, 255.0f);
}

inline int change(int value, float brightness, float contrast, float gamma)
{
    float v = value / 255.0f;
    v = v + brightness;
    v = ((v - 0.5f) * contrast) + 0.5f;
    v = std::pow(v, gamma);
    return kClamp(v * 255.0f, 0.0f, 255.0f);
}

template<class T>
inline T mix(T a, T b, T bias)
{
    return a + (b - a) * bias;
}

QColor mix(const QColor& c1, const QColor& c2, qreal bias);
