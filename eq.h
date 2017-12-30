/** EQ frontend for ShibachSuperEQ
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
#ifndef SSEQ_H
#define SSEQ_H

#include "common.h"
#include "effect.h"
#include "shibatch/equ.h"
#include "shibatch/paramlist.hpp"

#define BAND_COUNT 17

namespace Vrok {
class EffectSSEQ : public Effect
{
private:

    Property<float> _bands[BAND_COUNT];
    Property<float> _preamp;
    SuperEqState _sb_state;
    void *_sb_paramsroot;
    std::mutex _eq_setting_guard;
public:
    EffectSSEQ();
    ~EffectSSEQ();
    bool EffectRun(Buffer *out_buffer,
                   Buffer **in_buffer_set,
                   int buffer_count);
    void PropertyChanged(PropertyBase *property);
    bool BufferConfigChange(BufferConfig *config);
    Vrok::ComponentType ComponentType()
    {
        return Vrok::ComponentType::Effect;
    }
    Component *CreateSelf()
    {
        return new EffectSSEQ();
    }
    const char *ComponentName()
    {
        return "ShibachSuperEQ";
    }
    const char *Description()
    {
        return "Shibach Super EQ";
    }
    const char *Author()
    {
        return "Naoki Shibata, Madura A.";
    }
    const char *License()
    {
        return "GPL v2";
    }
};
}

#endif // SSEQ_H
