/** AudioOut (libao wrapper)
 * Copyright (C) Madura A.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111-1307, USA.
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>

#include "debug.h"

#include "component.h"
#include "componentmanager.h"

#include <mutex>

namespace Vrok {
    const int MAX_CHANNELS=8;

    template<typename T>
    void Clip(T& value, T low, T high)
    {
        if (value < low)
            value = low;
        else if (value > high)
            value = high;
    }

    template<typename T>
    inline T SmallValue() {
        return 0;
    }


    /// 'Small value' for floats (2^-24) - used for primitive underrun prevention. The value is pretty much arbitrary (allowing for 24-bit signals normalized to 1.0).
    template<>
    inline float SmallValue<float>() {
        return (1.0/16777216.0); // allows for 2^-24, should be enough for 24-bit DACs at least :)
    }

    /// 'Small value' for doubles (2^-24) - used for primitive underrun prevention. The value is pretty much arbitrary.
    template<>
    inline double SmallValue<double>() {
        return (1.0/16777216.0);
    }

    /**
     * Force "small enough" float value to zero
     */
    inline void Sanitize(float &value)
    {
        // real number?
        if (fabs(value) < SmallValue<float>())
            value = 0.f;
        // close to 0?
        const int val = *reinterpret_cast <const int *> (&value);
        if ((val & 0x7F800000) == 0 && (val & 0x007FFFFF) != 0)
            value = 0.f;
    }
    inline void Sanitize(double &value)
    {
        if (std::abs(value) < SmallValue<double>())
            value = 0.0;
    }

}

extern std::mutex lockxx;


