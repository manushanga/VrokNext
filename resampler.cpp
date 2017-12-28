
#include "resampler.h"

#define L_A 1

inline float sinc(float x)
{
    if (x == 0.0)
        return 1.0;

    return sin(x)/x;
}
inline float L(float x)
{
    if ( -L_A < x && x < L_A)
        return sinc(x) * sinc(x/L_A);
    else
        return 0.0;
}

inline float GetLSampleAt(float* buff, int ch, int nch, float at)
{
    float _at = std::floor(at);
    int start =_at - L_A + 1;

    if (_at < L_A)
        start = 0;

    int end = _at + L_A;
    float accum = 0.0;

    for (int i=start; i<= end;++i)
    {
        accum += buff[nch*i + ch] * L(at - i);
    }
    return accum;

}
#define L0(x, x0, x1, x2, x3) \
    ((x-x1) * (x-x2) * (x-x3))

#define L0_() \
    (-1 * -2 * -3)  /* (x0-x1) * (x0-x2) * (x0-x3) */

#define L1(x, x0, x1, x2, x3) \
    ((x-x0) * (x-x2) * (x-x3))

#define L1_() \
    (1 * -1 * -2) /* (x1-x0) * (x1-x2) * (x1-x3) */

#define L2(x, x0, x1, x2, x3) \
    ((x-x0) * (x-x1) * (x-x3))

#define L2_() \
    (2 * 1 * -1) /* (x2-x0) * (x2-x1) * (x2-x3) */

#define L3(x, x0, x1, x2, x3) \
    ((x-x0) * (x-x1) * (x-x2))

#define L3_() \
    (3 * 2 * 1) /* (x3-x0) * (x3-x1) * (x3-x2) */

float Lagrange(float tsrc, float tdest, float* tsrc_V)
{
    float tdest_V =
            L0(tdest, tsrc, tsrc+1, tsrc+2, tsrc+3) / L0_() * tsrc_V[0] +
                    L1(tdest, tsrc, tsrc+1, tsrc+2, tsrc+3) / L1_() * tsrc_V[1] +
                    L2(tdest, tsrc, tsrc+1, tsrc+2, tsrc+3) / L2_() * tsrc_V[2] +
                    L3(tdest, tsrc, tsrc+1, tsrc+2, tsrc+3) / L3_() * tsrc_V[3];

    return tdest_V;
}

Vrok::Resampler::Resampler()
{
    _out_samplerate.Set(44100);
    _out_samplerate.SetPropertyInfo(PropertyInfo{
            0.0, 192000.0,
            1.0,
            44100.0,
            {} });

    _mode.Set(5);

    _mode.SetPropertyInfo(PropertyInfo{
            0.0, 5.0,
            1.0,
            3.0,
            {"ZOH(low)","BLEP","Linear","BLAM","CUBIC","Sinc(best)"} });

    ComponentManager *c=ComponentManager::GetSingleton();
    c->RegisterComponent(this);
    c->RegisterProperty(this, "OutputSamplerate", &_out_samplerate);
    c->RegisterProperty(this, "InterpolatorMode", &_mode);
    _last = 0.0;
    _resamplers = nullptr;

    resampler_init();

}

bool Vrok::Resampler::EffectRun(Buffer *out_buffer, Buffer **in_buffer_set, int buffer_count)
{
    assert(buffer_count == 1);

    Buffer *src = in_buffer_set[0];

    assert(INTERNAL_BUFFER_SIZE >= src->GetBufferConfig()->channels * src->GetBufferConfig()->frames);
    int src_len = src->GetBufferConfig()->channels * src->GetBufferConfig()->frames;
    int src_flen = src->GetBufferConfig()->frames;
    int flen = _ratio * src->GetBufferConfig()->frames;

    int nch = src->GetBufferConfig()->channels;

    int sample_count = src->GetBufferConfig()->frames;
    int channel_count = nch;
    float *current_out = _buffer;
    float *current = src->GetData();
    int samples_out = 0;

    while( ( sample_count && resampler_get_free_count( _resamplers[0] ) ) || resampler_get_sample_count( _resamplers[0] ) )
    {
        while ( sample_count && resampler_get_free_count( _resamplers[0] ) )
        {
            for (int i = 0; i < channel_count; ++i)
                resampler_write_sample_float(_resamplers[i], current[i]);
            current += channel_count;
            --sample_count;
        }

        while (resampler_get_sample_count(_resamplers[0]))
        {
            for (int i = 0; i < channel_count; ++i)
            {
                current_out[i] = resampler_get_sample_float(_resamplers[i]);
                resampler_remove_sample(_resamplers[i], 1);
            }
            current_out += channel_count;
            samples_out ++;
        }
    }


    BufferConfig cfg;
    cfg.channels = src->GetBufferConfig()->channels;
    cfg.frames =/* src_flen;*/samples_out;//flen;// out_frames;
    cfg.samplerate = _out_samplerate.Get();
    out_buffer->Reset(&cfg);

    for (std::size_t i=0;i< /*src_len*/samples_out * nch;i++)
    {
        out_buffer->GetData()[i] = _buffer[i];
    }
    return true;
}

void Vrok::Resampler::PropertyChanged(Vrok::PropertyBase *property)
{
    if (!_resamplers)
        return;

    std::lock_guard<std::mutex> lg(_property_mutex);
    for (int i=0;i<_resamplers_count;i++)
    {
        resampler_set_quality(_resamplers[i], _mode.Get());
    }
}

bool Vrok::Resampler::BufferConfigChange(BufferConfig *config)
{
    std::lock_guard<std::mutex> lg(_property_mutex);
    DBG(0, "-----changed resampler rate " << _out_samplerate.Get() << " " << config->samplerate);
    _ratio =  double(config->samplerate) / double(_out_samplerate.Get())  ;

    if (_resamplers)
    {
        for (int i=0;i<_resamplers_count;i++)
        {
            resampler_clear( _resamplers[i] );
            resampler_delete( _resamplers[i] );
        }
    }

    delete[] _resamplers;

    _resamplers = new void*[config->channels];
    _resamplers_count = config->channels;

    for (int i=0;i<config->channels;i++)
    {
        _resamplers[i] = resampler_create();
        resampler_set_quality(_resamplers[i], _mode.Get());
        resampler_set_rate(_resamplers[i], _ratio );
    }
    return true;
}
