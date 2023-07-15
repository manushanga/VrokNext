/** AudioOut (libao wrapper)
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

#include <ao/ao.h>
#include <atomic>

#include "driver.h"

namespace vrok {

class DriverAudioOut : public Driver {
private:
    std::atomic<bool> _new_resource;
    std::atomic<double> _volume;
    ao_device *_ao_device;
    int _device_id;

protected:
public:
    DriverAudioOut();
    virtual ~DriverAudioOut() { }
    std::vector<DeviceInfo> GetDeviceInfo();
    std::string GetDefaultDevice();
    bool SetDevice(std::string device);
    bool BufferConfigChange(BufferConfig *config);
    bool DriverRun(Buffer *buffer);
    void SetVolume(double volume);
    vrok::ComponentType ComponentType() { return vrok::ComponentType::Driver; }
    Component *CreateSelf() { return new DriverAudioOut(); }
    const char *ComponentName() { return "AudioOut Driver"; }
    const char *Description() { return "libao wrapper"; }
    const char *Author() { return "Madura A."; }
    const char *License() { return "GPL v2"; }
};
}
