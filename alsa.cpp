#include "alsa.h"

#include <cmath>

#define PERIOD_SIZE 128

#define X32MUL ( (1U<<31) -1 )
#define X24MUL ( (1U<<23) -1 )
#define X16MUL ( (1U<<15) -1 )
#define X8MUL ( (1U<<7) -1 )

#define X24_GETI(buffer, i) ( (char*) buffer )[i*3]
#define X24_WRITE(buffer, i, X32) memcpy( &(X24_GETI(buffer, i)), &(X32), 3 )

Vrok::DriverAlsa::DriverAlsa() :
    _handle(nullptr),
    _params(nullptr),
    _buffer(nullptr)
{
}

bool Vrok::DriverAlsa::BufferConfigChange(BufferConfig *config)
{
    if (_handle)
    {
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
    snd_pcm_sw_params_current (_handle, swparams);
    snd_pcm_sw_params_set_start_threshold (_handle, swparams, PERIOD_SIZE);
    snd_pcm_sw_params (_handle, swparams);
    snd_pcm_sw_params_free(swparams);
*/
    snd_pcm_hw_params_alloca(&_params);
    snd_pcm_hw_params_any(_handle, _params);
    snd_pcm_hw_params_set_access(_handle, _params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_format_mask_t *mask;
    snd_pcm_format_mask_alloca(&mask);
    snd_pcm_hw_params_get_format_mask(_params, mask);

    if (_buffer)
    {
        delete[] _buffer;
        _buffer = nullptr;
    }

    if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S32))
    {
        DBG(1,"bit depth is 32");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S32);
        _buffer = new char[ 4 * config->frames * config->channels ];
        _multiplier = X32MUL;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S24))
    {
        DBG(1,"bit depth is 24");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S24);
        _buffer = new char[ 3 * config->frames * config->channels ];
        _multiplier = X24MUL;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S16))
    {
        DBG(1,"bit depth is 16");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S16);
        _buffer = new char[ 2 * config->frames * config->channels ];
        _multiplier = X16MUL;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S8))
    {
        DBG(1,"bit depth is 8");
        snd_pcm_hw_params_set_format(_handle, _params, SND_PCM_FORMAT_S8);
        _buffer = new char[ 1 * config->frames * config->channels ];
        _multiplier = X8MUL;
    } else
    {
        throw std::runtime_error("unsupported native hardware format");
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

    return true;
}

bool Vrok::DriverAlsa::DriverRun(Buffer *buffer)
{
    int ret;
    int ibuffer[8192*8];
    int frames = buffer->GetBufferConfig()->frames * buffer->GetBufferConfig()->channels;

    switch (_multiplier)
    {
    case X32MUL:
    {
        int32_t *ibuffer = (int32_t *) _buffer;
        for (int i=0;i< frames;i++)
        {
            double val = buffer->GetData()[i];
            Vrok::Clip(val,-1.0,1.0);
            ibuffer[i] = val * _multiplier;
        }
        break;
    }
    case X24MUL:
    {

        for (int i=0;i< frames;i++)
        {
            double val = buffer->GetData()[i];
            Vrok::Clip(val,-1.0,1.0);
            int ival = val * _multiplier;
            X24_WRITE(_buffer, i, ival);
        }
        break;
    }
    case X16MUL:
    {
        int16_t *ibuffer = (int16_t *) _buffer;
        for (int i=0;i< frames;i++)
        {
            double val = buffer->GetData()[i];
            Vrok::Clip(val,-1.0,1.0);
            ibuffer[i] = val * _multiplier;
        }
        break;
    }
    case X8MUL:
    {
        int8_t *ibuffer = (int8_t *) _buffer;
        for (int i=0;i< frames;i++)
        {
            double val = buffer->GetData()[i];
            Vrok::Clip(val,-1.0,1.0);
            ibuffer[i] = val * _multiplier;
        }
        break;
    }
    default:
        throw std::runtime_error("invalid hardware format");
    }

    ret = snd_pcm_writei(_handle, _buffer, buffer->GetBufferConfig()->frames);

    if (ret == -EPIPE || ret == -EINTR || ret == -ESTRPIPE)
    {
        DBG(1,"trying to recover");
        if ( snd_pcm_recover(_handle, ret, 0) < 0 )
        {
            WARN(9,"recover failed for "<<ret);
        }
    } else if (ret < 0 && ret != -EAGAIN)
    {
        WARN(9,"write error "<<ret);
    }

    return true;
}
