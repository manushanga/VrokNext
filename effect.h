/** Effect interface
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

#include <atomic>

#include "bgpoint.h"
#include "vumeter.h"

namespace Vrok {
class Effect : public BufferGraph::Point, public Component
{
private:
    BufferConfig _input_bc;
    bool _first_run;
protected:
    std::atomic<bool> _work;
public:

    Effect();
    virtual ~Effect() {}
    virtual bool EffectRun(Buffer *out_buffer,
                           Buffer **in_buffer_set,
                           int buffer_count)=0;
    virtual bool BufferConfigChange(BufferConfig *config) { return true; }
    void Run();
    const BufferConfig& GetOldBufferConfig() { return _input_bc; };
    std::vector<VUMeter *> GetMeters()
    {
        return std::vector<VUMeter *>();
    }
};
}
