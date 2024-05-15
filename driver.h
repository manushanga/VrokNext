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
#include "vumeter.h"

namespace vrok {

class Driver : public BufferGraph::Point, public Component {
private:
    std::atomic<bool> _new_resource;

protected:
    std::atomic<bool> _work;
    std::atomic<real_t> _volume;
    VUMeter _meter;
    BufferConfig _input_bc;
    bool _first_run;

    const BufferConfig &GetOldBufferConfig() { return _input_bc; }

public:
    class DeviceInfo {
    public:
        std::string name;
        void *user_data;
        bool operator==(const DeviceInfo &device_info) {
            return name == device_info.name && user_data == device_info.user_data;
        }
        bool operator==(DeviceInfo &device_info) {
            return name == device_info.name && user_data == device_info.user_data;
        }
    };
    Driver();
    virtual ~Driver() { }
    virtual std::vector<DeviceInfo> GetDeviceInfo() = 0;
    virtual std::string GetDefaultDevice() = 0;
    virtual bool SetDevice(std::string device) = 0;
    virtual bool BufferConfigChange(BufferConfig *config) = 0;
    virtual bool DriverRun(Buffer *buffer) = 0;
    virtual void SetVolume(real_t volume);
    virtual std::vector<VUMeter *> GetMeters();
    void Run();

    // volume in decibels
};
}
