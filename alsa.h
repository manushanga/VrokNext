/** ALSA driver
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

#include <unistd.h>
#include <alsa/asoundlib.h>
#include <sys/inotify.h>
#include <samplerate.h>

#include "driver.h"

namespace Vrok {

    class DriverAlsa : public Driver
    {
    private:
        uint32_t _multiplier;
        snd_pcm_t *_handle;
        snd_pcm_hw_params_t *_params;
    protected:
    public:
        DriverAlsa();
        virtual ~DriverAlsa() {}
        bool BufferConfigChange(BufferConfig *config);
        bool DriverRun(Buffer *buffer);

        Vrok::ComponentType ComponentType()
        {
            return Vrok::ComponentType::Driver;
        }
        Component *CreateSelf()
        {
            return new DriverAlsa();
        }
        const char *ComponentName()
        {
            return "ALSA Driver";
        }
        const char *Description()
        {
            return "ALSA wrapper";
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
