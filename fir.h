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
#pragma once

#include "common.h"
#include "effect.h"
#include "tapdistortion.h"
namespace Vrok {
class EffectFIR : public Effect
{
private:
    float *_buffer;
    Property<float> _blend,_drive,_dry_vol,_wet_vol;
    Property<float> _lp_freq,_hp_freq;
    float _f32_dry_vol,_f32_wet_vol;
    TapDistortion tap;
    biquad_d2 _lp[2][4];
    biquad_d2 _hp[2][2];
    TapDistortion _dist[2];
public:
    EffectFIR();
    bool EffectRun(Buffer *out_buffer,
                   Buffer **in_buffer_set,
                   int buffer_count);
    void PropertyChanged(PropertyBase *property);
    Vrok::ComponentType ComponentType()
    {
        return Vrok::ComponentType::Effect;
    }
    Component *CreateSelf()
    {
        return new EffectFIR();
    }
    const char *ComponentName()
    {
        return "FIR filter";
    }
    const char *Description()
    {
        return "FIR filter";
    }
    const char *Author()
    {
        return "Madura A.";
    }
    const char *License()
    {
        return "GPL v2";
    }
};
}
