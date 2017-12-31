 
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
#include "ringbuffer.h"

#include "resampler/resampler.h"

namespace Vrok {
class Resampler : public Effect
{
private:
    static const int INTERNAL_BUFFER_SIZE = 8192*4;

    Property<int> _out_samplerate;
    Property<int> _mode;

    void** _resamplers;
    int _resamplers_count;
    std::mutex _property_mutex;

    float _ratio;
    alignas(64) float _buffer[INTERNAL_BUFFER_SIZE];

public:
    Resampler();
    bool EffectRun(Buffer *out_buffer,
                   Buffer **in_buffer_set,
                   int buffer_count);
    void PropertyChanged(PropertyBase *property);
    bool BufferConfigChange(BufferConfig *config);
    Vrok::ComponentType ComponentType()
    {
        return Vrok::ComponentType::Effect;
    }
    std::vector<VUMeter *> GetMeters()
    {
        std::vector<VUMeter *> meters;
        return meters;
    }
    Component *CreateSelf()
    {
        return new Resampler();
    }
    const char *ComponentName()
    {
        return "Resampler";
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
