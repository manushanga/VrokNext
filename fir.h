/** FIR Filter
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
#ifndef FIR_H
#define FIR_H

#include "effect.h"
#include "tapdistortion.h"
namespace Vrok {
class EffectFIR : public Effect
{
private:
    float *_buffer;
    TapDistortion tap;
    biquad_d2 lp[2][4];
    biquad_d2 hp[2][2];
    TapDistortion dist[2];
public:
    EffectFIR();
    bool EffectRun(Buffer *out_buffer,
                   Buffer **in_buffer_set,
                   int buffer_count);
};
}

#endif // FIR_H
