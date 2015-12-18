#pragma once

#include <atomic>

#include "common.h"
#include "buffer.h"

namespace Vrok
{
class VUMeter
{
    std::atomic<double> _clip[MAX_CHANNELS];
    std::atomic<double> _value[MAX_CHANNELS];
    int _count_over[MAX_CHANNELS];
    double _falloff, _clip_falloff;
public:
    VUMeter();
    double GetValue(int channel) { return _value[channel]; }
    double GetClipping(int channel) { return _clip[channel]; }
    void Process(Buffer *buffer);
    void SetBufferConfig(BufferConfig *bc);
};
}
