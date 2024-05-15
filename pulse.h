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

#include <atomic>
#include <pulse/simple.h>
#include <unistd.h>

#include "driver.h"

namespace vrok {

class DriverPulse : public Driver {
private:
    pa_simple *_p;
    std::string _device;

protected:
public:
    DriverPulse();
    virtual ~DriverPulse() { }
    bool SetDevice(std::string device);
    std::vector<DeviceInfo> GetDeviceInfo();
    std::string GetDefaultDevice();
    bool BufferConfigChange(BufferConfig *config);
    bool DriverRun(Buffer *buffer);
    vrok::ComponentType ComponentType() { return vrok::ComponentType::Driver; }
    Component *CreateSelf() { return new DriverPulse(); }
    const char *ComponentName() { return "PulseAudio Driver"; }
    const char *Description() { return ""; }
    const char *Author() { return "Madura A."; }
    const char *License() { return "GPL v2"; }
};
}
