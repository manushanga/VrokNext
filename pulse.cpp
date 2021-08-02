#include "pulse.h"
#include "common.h"
#include "debug.h"

#include <bits/stdint-intn.h>
#include <fenv.h>
#include <cmath>
#include <pulse/sample.h>
#include <pulse/simple.h>

#define PERIOD_SIZE 128

#define X32MUL ((int32_t)( (1U<<31) -1 ))
#define X24MUL ((int32_t)( (1U<<23) -1 ))
#define X16MUL ((int16_t)( (1U<<15) -1 ))
#define X8MUL ((int8_t)( (1U<<7) -1 ))

#define X24_GETI(buffer, i) ( (char*) buffer )[i*3]
#define X24_WRITE(buffer, i, X32) memcpy( &(X24_GETI(buffer, i)), &(X32), 3 )
#define FLT_TO_INT(__flt, __scale, __T) ( __T( ((__flt)*(__scale)) ) )

Vrok::DriverPulse::DriverPulse() :
    _p(nullptr),
    _device("pulse")
{
}

bool Vrok::DriverPulse::SetDevice(std::string device)
{
    _device = device;
    return true;
}

std::vector<Vrok::Driver::DeviceInfo> Vrok::DriverPulse::GetDeviceInfo()
{
    std::vector<DeviceInfo> info;
    DeviceInfo devinfo;
    devinfo.name = "pulse";
    devinfo.user_data = (void*) nullptr;
    info.push_back(devinfo);
    return info;
}

std::string Vrok::DriverPulse::GetDefaultDevice()
{
    return "pulse";
}

bool Vrok::DriverPulse::BufferConfigChange(BufferConfig *config)
{
    config->Print();

    if (GetOldBufferConfig().samplerate != config->samplerate || GetOldBufferConfig().channels != config->channels)
    {
        if (_p)
        {
            pa_simple_free(_p);
            _p = nullptr; 
        }
        pa_simple *s;
        pa_sample_spec ss;
        ss.format = PA_SAMPLE_FLOAT32;
        ss.channels = config->channels;
        ss.rate = config->samplerate;
        _p = pa_simple_new(NULL,               // Use the default server.
                  "Vrok",           // Our application's name.
                  PA_STREAM_PLAYBACK,
                  NULL,               // Use the default device.
                  "Music",            // Description of our stream.
                  &ss,                // Our sample format.
                  NULL,               // Use default channel map
                  NULL,               // Use default buffering attributes.
                  NULL               // Ignore error code.
                  );
        if (_p == nullptr)
        {
            return false;
        }
    }
    return true;
}

bool Vrok::DriverPulse::DriverRun(Buffer *buffer)
{
    int ret;
    int frames = buffer->GetBufferConfig()->frames * buffer->GetBufferConfig()->channels;
    if (buffer->getBufferType() == Buffer::Type::StreamStart)
    {
        DBG(0,"btype:start/end");
        //snd_pcm_drop(_handle);
        //snd_pcm_prepare(_handle);
        int error=0;
        pa_simple_flush(_p, &error);
    }
    else if (buffer->getBufferType() == Buffer::Type::StreamPause)
    {
        //snd_pcm_drop(_handle);
        //snd_pcm_prepare(_handle);
        int error=0;
        pa_simple_flush(_p, &error);
        return true;
    }
    else if (buffer->getBufferType() == Buffer::Type::StreamResume)
    {
    }
    int error=0; 
    pa_simple_write(_p, (void*) buffer->GetData(), frames * sizeof(float), &error);
    
    return (error == 0);
}
