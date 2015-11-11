#include "alsa.h"

#define PERIOD_SIZE 128

Vrok::DriverAlsa::DriverAlsa() :
    _handle(nullptr),
    _params(nullptr)
{
}

bool Vrok::DriverAlsa::BufferConfigChange(BufferConfig *config)
{
    if (_handle)
    {
        snd_pcm_pause(_handle, true);
        snd_pcm_drop(_handle);
        snd_pcm_close(_handle);
        _handle = nullptr;
        _params = nullptr;
    }
    if (snd_pcm_open(&_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        throw std::runtime_error("Alsa:init: failed to open pcm");
    }
/*
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_malloc(&swparams);
    snd_pcm_sw_params_current (handle, swparams);
    snd_pcm_sw_params_set_start_threshold (handle, swparams, VPBUFFER_FRAMES - PERIOD_SIZE);
    snd_pcm_sw_params (handle, swparams);
    snd_pcm_sw_params_free(swparams);
*/
    snd_pcm_hw_params_alloca(&_params);
    snd_pcm_hw_params_any(_handle, _params);
    snd_pcm_hw_params_set_access(_handle, _params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_format_mask_t *mask;
    snd_pcm_format_mask_alloca(&mask);
    snd_pcm_hw_params_get_format_mask(_params, mask);

    if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S32))
    {
        DBG("bit depth is 32");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S32);
        _multiplier = (1<<31) -1 ;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S24))
    {
        DBG("bit depth is 24");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S24);
        _multiplier = (1<<23) -1;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S16))
    {
        DBG("bit depth is 16");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S16);
        _multiplier = (1<<15) -1;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S8))
    {
        DBG("bit depth is 8");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S8);
        _multiplier = (1<<7) -1;;
    } else
    {
        throw std::runtime_error("unsupported native format");
    }

    snd_pcm_hw_params_set_channels(_handle, _params, config->channels);
    snd_pcm_hw_params_set_rate(_handle, _params, config->samplerate, 0);
    snd_pcm_hw_params_set_period_size(_handle, _params, PERIOD_SIZE, 0);

    if (snd_pcm_hw_params(_handle, _params) < 0)
    {
        throw std::runtime_error("Alsa:init: failed to set pcm params");
    }

    snd_pcm_hw_params_current(_handle, _params);
    int dir;
    //snd_pcm_hw_params_get_rate(_params, &out_srate, &dir);

    snd_pcm_pause(_handle, false);

    return true;
}

bool Vrok::DriverAlsa::DriverRun(Buffer *buffer)
{
    int ret;
    int ibuffer[8192*8];
    int frames = buffer->GetBufferConfig()->frames * buffer->GetBufferConfig()->channels;
    for(int i=0;i< frames;i++)
    {
        ibuffer[i] = buffer->GetData()[i] * _multiplier;
    }
    ret = snd_pcm_writei(_handle, ibuffer, buffer->GetBufferConfig()->frames);

    if (ret == -EPIPE || ret == -EINTR || ret == -ESTRPIPE)
    {
        DBG("trying to recover");
        if ( snd_pcm_recover(_handle, ret, 0) < 0 )
        {
            DBG("recover failed for "<<ret);
        }
    } else if (ret < 0 && ret != -EAGAIN)
    {
        DBG("write error "<<ret);
    }
    return true;
}
