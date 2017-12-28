#pragma once

#include <math.h>

float __FM_reciprocalf( float x );


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

#define FM_divf(x, y) \
    ( x * FM_reciprocalf(y) )
