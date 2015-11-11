#include "audioout.h"

Vrok::DriverAudioOut::DriverAudioOut()
{
    ao_shutdown();
    ao_initialize();

    ao_sample_format sformat;

    sformat.channels=GetBufferConfig()->channels;
    sformat.rate=GetBufferConfig()->samplerate;
    sformat.bits=16;
    sformat.byte_format=AO_FMT_NATIVE;
    sformat.matrix=0;

    _ao_device=ao_open_live(ao_default_driver_id(),&sformat,NULL);
}
bool Vrok::DriverAudioOut::BufferConfigChange(BufferConfig *config)
{
    if (*GetBufferConfig() != *config) {
        if (_ao_device)
        {
            ao_close(_ao_device);
            ao_shutdown();
            ao_initialize();

        }
        ao_sample_format sformat;

        sformat.channels=config->channels;
        sformat.rate=config->samplerate;
        sformat.bits=16;
        sformat.byte_format=AO_FMT_NATIVE;
        sformat.matrix=0;

        _ao_device=ao_open_live(ao_default_driver_id(),&sformat,NULL);

        DBG("o "<<config->channels);

        DBG("o "<<config->samplerate);
    }
    return true;
}
bool Vrok::DriverAudioOut::DriverRun(Buffer *buffer)
{
    //DBG(buffer);
    DBG("arrival: "<<buffer->GetWatch().Stop());

    uint16_t cbuf[8192*4];

    int samples=GetBufferConfig()->channels * GetBufferConfig()->frames;

    for (int i=0;i<samples;i++)
    {
        cbuf[i]=(uint16_t)(buffer->GetData()[i]*32767.0);
    }

    ao_play(_ao_device,(char *) &cbuf[0],samples*sizeof(uint16_t));


    return true;
}
