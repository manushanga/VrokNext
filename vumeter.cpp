#include <cmath>
#include <tuple>

#include "vumeter.h"
#define VIS_BUF_COUNT 64

vrok::VUMeter::VUMeter(std::string name) : _name(name), _falloff(0), _clip_falloff(0), _active(true) {
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        _value[i] = 0;
        _clip[i] = 0;
        _count_over[i] = 0;
    }
    for (int i = 0; i < VIS_BUF_COUNT; i++) {
    }
}

void vrok::VUMeter::GetValues(int channel, double &value, double &clip) {
    value = _value[channel];
    clip = _clip[channel];
}

double vrok::VUMeter::GetValue(int channel) {
    return _value[channel].load();
}

double vrok::VUMeter::GetClip(int channel) {
    return _clip[channel].load();
}

void vrok::VUMeter::Process(Buffer *buffer) {
    auto bc = buffer->GetBufferConfig();
    for (int ch = 0; ch < bc->channels; ++ch) {
        auto val = _value[ch].load();
        val *= std::pow(_falloff, bc->frames);
        Sanitize(val);
        _value[ch] = val;

        // Same for clip level (using different fade constant)
        auto clip = _clip[ch].load();
        clip *= std::pow(_clip_falloff, bc->frames);
        Sanitize(clip);
        _clip[ch] = clip;

        if (!_active)
            continue;

        for (int i = 0; i < bc->frames; ++i) {
            double val = std::max<double>(_value[ch].load(), fabs(buffer->GetData()[bc->channels * i + ch]));
            _value[ch] = val;
            if (val > 1.0)
                ++_count_over[ch];
            else
                _count_over[ch] = 0;

            if (_count_over[ch] >= 3)
                _clip[ch] = 1.0;
        }
    }
}

void vrok::VUMeter::SetBufferConfig(BufferConfig *bc) {
    const double time_20dB = 0.3;
    // 20dB = 10x +/- --> 0.1 = pow(falloff, sample_rate * time_20dB) = exp(sample_rate * ln(falloff))
    // ln(0.1) = sample_rate * ln(falloff)
    _falloff = std::pow(0.1, 1 / (bc->samplerate * time_20dB));
    _clip_falloff = _falloff;
}
