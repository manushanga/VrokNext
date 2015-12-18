/** Audio Driver interface
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

#include "bgpoint.h"

namespace Vrok {

    class Driver : public BufferGraph::Point, public Component
    {
    private:
        atomic<bool> _new_resource;
    protected:
        atomic<bool> _work;
    public:
        Driver();
        virtual ~Driver() {}
        virtual bool BufferConfigChange(BufferConfig *config)=0;
        virtual bool DriverRun(Buffer *buffer)=0;
        virtual void setVolume(double volume) = 0;
        void Run();

        // volume in decibels

    };
}

