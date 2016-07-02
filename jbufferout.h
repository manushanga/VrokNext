#pragma once

#include "driver.h"

namespace Vrok {

class DriverJBufferOut : public Driver
{
private:


protected:
public:
    class Events
    {
    public:
        virtual void OnBuffer(Buffer* buffer) = 0;
        virtual void OnBufferConfigChange(int frames, int samplerate, int channels) = 0;
    };
    DriverJBufferOut();
    std::vector<DeviceInfo> GetDeviceInfo();
    std::string GetDefaultDevice();
    bool SetDevice(std::string device);
    void ThreadStart();
    void ThreadEnd();
    virtual ~DriverJBufferOut();
    bool BufferConfigChange(BufferConfig *config);
    bool DriverRun(Buffer *buffer);

    Vrok::ComponentType ComponentType()
    {
        return Vrok::ComponentType::Driver;
    }
    Component *CreateSelf()
    {
        return new DriverJBufferOut();
    }
    const char *ComponentName()
    {
        return "JavaBuffer Driver";
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
