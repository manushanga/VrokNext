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

#define BAR_COUNT 18

namespace Vrok {
class EffectSSEQ : public Effect
{
private:

    Property<float> _bands[BAR_COUNT];
    Property<float> _preamp;
    SuperEqState _sb_state;
    void *_sb_paramsroot;

public:
    EffectSSEQ();
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
        return new EffectSSEQ();
    }
    const char *ComponentName()
    {
        return "SSEQ";
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
