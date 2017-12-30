#pragma once

#include <math.h>

inline static float __FM_reciprocalf( float x )
{
    union {
        float dbl;
        unsigned uint;
    } u;
    u.dbl = x;
    u.uint = ( 0xbe6eb3beU - u.uint ) >> (unsigned char)1;
    // pow( x, -0.5 )
    u.dbl *= u.dbl;                 // pow( pow(x,-0.5), 2 ) = pow( x, -1 ) = 1.0 / x
    return u.dbl;
}

inline static float __FM_clampf(float value, float low, float high)
{
    const float t = value < low ? low : value;
    return high < t ? high : t;
}

inline static double __FM_clampd(double value, double low, double high)
{
    const double t = value < low ? low : value;
    return high < t ? high : t;
}

#define FM_sinf(x) \
    sinf(x)

#define FM_cosf(x) \
    cosf(x)

#define FM_absf(x) \
    fabs(x)

#define FM_fmodf(x, y) \
    fmodf(x, y)

#define FM_fractionalf(x) \
    (x - ((long) x))

#define FM_reciprocalf(x) \
    (__FM_reciprocalf(x))

#define FM_clampf(x, low, high) \
    (__FM_clampf(x, low, high))

#define FM_clampd(x, low, high) \
    (__FM_clampd(x, low, high))

#define FM_signf(x) \
    ( (int) ((0.0 < x) - (x < 0.0) ) )

#define FM_divf(x, y) \
    ( x * FM_reciprocalf(y) )
