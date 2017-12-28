#include "fastmath.h"

float __FM_reciprocalf( float x ) {
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