#pragma once

#include <samplerate.h>

#include "driver.h"

namespace Vrok {

class DriverJBufferOut : public Driver
{
protected:
public:
    class Events
    {
    public:
        virtual void OnBuffer(double* buffer) = 0;
        virtual void OnBufferConfigChange(int frames, int samplerate, int channels) = 0;
    };

    DriverJBufferOut();
    std::vector<DeviceInfo> GetDeviceInfo();
    std::string GetDefaultDevice();
    void SetEvents(Events* events);
    bool SetDevice(std::string device);
    void ThreadStart();
    void ThreadEnd();
    virtual ~DriverJBufferOut();
    bool BufferConfigChange(BufferConfig *config);
    bool DriverRun(Buffer *buffer);
    void SetOutputSamplerate(int samplerate);

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
private:
    static const int INTERNAL_BUFFER_SIZE = 17000;
    Events* m_events;
    int _out_sr;
    int _in_sr;
    SRC_STATE *_current_state;
    SRC_DATA _sr_data;

    float _buffer[INTERNAL_BUFFER_SIZE];
    float _out_buffer[INTERNAL_BUFFER_SIZE];
    double _out_dbl_buffer[INTERNAL_BUFFER_SIZE];
};

}
