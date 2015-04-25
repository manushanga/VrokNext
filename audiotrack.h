/** AudioTrack output for Android
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
#ifndef AUDIOTRACK_H
#define AUDIOTRACK_H

#include "driver.h"

namespace Vrok {

    class DriverAudioTrack : public Driver
    {
    private:
        atomic<bool> _new_resource;
        void initAudioTrack(BufferConfig *config);
        void finiAudioTrack(BufferConfig *config);
        bool _init;
    protected:
    public:
        DriverAudioTrack();
        void ThreadStart();
        void ThreadEnd();
        virtual ~DriverAudioTrack();
        bool BufferConfigChange(BufferConfig *config);
        bool DriverRun(Buffer *buffer);
        
        Vrok::ComponentType ComponentType()
        {
            return Vrok::ComponentType::Driver;
        }
        Component *CreateSelf()
        {
            return new DriverAudioTrack();
        }
        const char *ComponentName()
        {
            return "AudioTrack  Driver";
        }
        const char *Description()
        {
            return "Android AudioTrack";
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
#endif // AUDIOOUT_H
