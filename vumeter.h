#pragma once

#include <atomic>

#include "buffer.h"
#include "common.h"

namespace vrok {
class VUMeter {
    std::atomic<double> _clip[MAX_CHANNELS];
    std::atomic<double> _value[MAX_CHANNELS];
    int _count_over[MAX_CHANNELS];
    double _falloff, _clip_falloff;
    std::atomic<bool> _active;
    std::vector<Buffer *> _free_bufs;
    std::string _name;

public:
    VUMeter(std::string name);
    void GetValues(int channel, double &value, double &clip);
    double GetValue(int channel);
    double GetClip(int channel);
    void Process(Buffer *buffer);
    void SetBufferConfig(BufferConfig *bc);
    void SetActive(bool active) { _active = active; }
    bool GetActive() { return _active; }
};
}
