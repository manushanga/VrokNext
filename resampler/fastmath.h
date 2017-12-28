#pragma once

#include <math.h>

#define FM_sinf(x) \
    sinf(x)

#define FM_cosf(x) \
    cosf(x)

#define FM_absf(x) \
    fabs(x)

#define FM_fmodf(x, y) \
    fmodf(x, y)

#define FM_fractional(x) \
    (x - ((long) x))
