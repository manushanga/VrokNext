
#include "resampler.h"

Vrok::Resampler::Resampler()
{
    _out_samplerate.Set(44100);

    ComponentManager *c=ComponentManager::GetSingleton();
    c->RegisterComponent(this);
    c->RegisterProperty(this, "output_samplerate", &_out_samplerate);
}

bool Vrok::Resampler::EffectRun(Buffer *out_buffer, Buffer **in_buffer_set, int buffer_count)
{
    assert(buffer_count == 1);

    Buffer *src = in_buffer_set[0];
    SRC_DATA data_copy = _sr_data;

    assert(INTERNAL_BUFFER_SIZE >= src->GetBufferConfig()->channels * src->GetBufferConfig()->frames);
    int len = src->GetBufferConfig()->channels * src->GetBufferConfig()->frames;
    for (int i=0;i<len;i++)
    {
        _buffer[i] = float(src->GetData()[i]);
    }

    data_copy.data_in = &_buffer[0];
    data_copy.data_out = &_out_buffer[0];

    data_copy.end_of_input = 0;
    data_copy.output_frames = src->GetBufferConfig()->frames * _sr_data.src_ratio *2;

    assert(INTERNAL_BUFFER_SIZE >= data_copy.output_frames);

    data_copy.output_frames_gen = 1;
    int out_frames=0;

    data_copy.input_frames = src->GetBufferConfig()->frames ;

    while (data_copy.output_frames_gen > 0) {
        int ret = src_process(_current_state,&data_copy);
        if (ret != 0)
            WARN(0, src_strerror(ret));

        data_copy.input_frames -= data_copy.input_frames_used;
        data_copy.data_in += data_copy.input_frames_used* src->GetBufferConfig()->channels;
        out_frames+=data_copy.output_frames_gen;
        //std::cout<< out_frames << data_copy.output_frames  <<std::endl;
    }


    len = out_frames * src->GetBufferConfig()->channels;
    assert( len < INTERNAL_BUFFER_SIZE);

    BufferConfig cfg;
    cfg.channels = src->GetBufferConfig()->channels;
    cfg.frames = out_frames;
    cfg.samplerate = _out_samplerate.Get();
    out_buffer->Reset(&cfg);

    //int len = src->GetBufferConfig()->frames *  src->GetBufferConfig()->channels;
    for (int i=0;i<len;i++)
    {
        out_buffer->GetData()[i] = double(_out_buffer[i]);
        //out_buffer->GetData()[i] = src->GetData()[i];

    }

    return true;
}

void Vrok::Resampler::PropertyChanged(Vrok::PropertyBase *property)
{
}

bool Vrok::Resampler::BufferConfigChange(BufferConfig *config)
{
    DBG(0,"-----changed resamplere");
    _sr_data.src_ratio = double(_out_samplerate.Get())/ double(config->samplerate);
    int err=0;
    _current_state = src_new(SRC_SINC_FASTEST, config->channels, &err);
    DBG(0,"asdd"<<_current_state);
    return true;
}
