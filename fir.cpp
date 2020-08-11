#include "fir.h"

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 2000 Hz

* 0 Hz - 90 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.986475904421452 dB

* 200 Hz - 1000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.38887353005501 dB

*/

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 2000 Hz

* 0 Hz - 90 Hz
  gain = 2
  desired ripple = 5 dB
  actual ripple = 3.576397969398726 dB

* 200 Hz - 1000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -41.302631617010256 dB

*/

#define FIR_LEN 90

Vrok::EffectFIR::EffectFIR() :
    _meter("out"),
    _buffer(new float[(FIR_LEN+GetBufferConfig()->frames)* GetBufferConfig()->channels])
{
    int len=(FIR_LEN+GetBufferConfig()->frames)* GetBufferConfig()->channels;
    for (int i=0;i<len;i++)
    {
        _buffer[i] = 0.0;
    }
    _dist[0].activate();
    _dist[1].activate();
    ComponentManager *c=ComponentManager::GetSingleton();

    c->RegisterComponent(this);
    c->RegisterProperty(this,"blend",&_blend);
    c->RegisterProperty(this,"drive",&_drive);

    c->RegisterProperty(this,"lp_freq",&_lp_freq);
    c->RegisterProperty(this,"hp_freq",&_hp_freq);
    c->RegisterProperty(this,"dry_vol",&_dry_vol);
    c->RegisterProperty(this,"wet_vol",&_wet_vol);

    _blend.Set(9);
    _drive.Set(1);

    _lp_freq.Set(150.0f);
    _hp_freq.Set(50.0f);

    _wet_vol.Set(0.5);
    _dry_vol.Set(0.5);

    _f32_dry_vol = _dry_vol.Get();
    _f32_wet_vol = _wet_vol.Get();

    _lp[0][0].set_lp_rbj(_lp_freq.Get(), 0.707, (float)GetBufferConfig()->samplerate);
    _lp[0][1].copy_coeffs(_lp[0][0]);
    _lp[0][2].copy_coeffs(_lp[0][0]);
    _lp[0][3].copy_coeffs(_lp[0][0]);
    _lp[1][0].copy_coeffs(_lp[0][0]);
    _lp[1][1].copy_coeffs(_lp[0][0]);
    _lp[1][2].copy_coeffs(_lp[0][0]);
    _lp[1][3].copy_coeffs(_lp[0][0]);
    _hp[0][0].set_hp_rbj(_hp_freq.Get(), 0.707, (float)GetBufferConfig()->samplerate);
    _hp[0][1].copy_coeffs(_hp[0][0]);
    _hp[1][0].copy_coeffs(_hp[0][0]);
    _hp[1][1].copy_coeffs(_hp[0][0]);
    _dist[0].set_sample_rate(GetBufferConfig()->samplerate);
    _dist[0].set_params(_blend.Get(),_drive.Get());
    _dist[1].set_sample_rate(GetBufferConfig()->samplerate);
    _dist[1].set_params(_blend.Get(),_drive.Get());


}

bool Vrok::EffectFIR::EffectRun(Buffer *out_buffer, Buffer **in_buffer_set, int buffer_count)
{
    BufferConfig *bc=GetBufferConfig();
    int fir_len=FIR_LEN*bc->channels;
    int len=bc->frames*bc->channels;

    for (int i=0;i<len;i++)
    {
        out_buffer->GetData()[i] = (in_buffer_set[0]->GetData()[i]);
    }
    /*double *proc=in_buffer_set[0]->GetData();
    double *proc_out=out_buffer->GetData();
    proc_out[0]=proc[0];
    for (int j=0;j<bc->frames;j++) {

        for (int i=0;i<bc->channels;i++)
        {
            proc_out[i] = proc[i];
        }
        for (int i=0;i<bc->channels;i++)
        {
            proc_out[i] = _lp[i][1].process(_lp[i][0].process(proc_out[i]));

            proc_out[i] = _dist[i].process(proc_out[i]);

            proc_out[i] = _lp[i][2].process(_lp[i][3].process(proc_out[i]));

            proc_out[i] = (proc_out[i]*_f32_wet_vol + proc[i])*_f32_dry_vol;
        }
        proc+=bc->channels;
        proc_out+=bc->channels;
    }
    _lp[0][0].sanitize();
    _lp[1][0].sanitize();
    _lp[0][1].sanitize();
    _lp[1][1].sanitize();
    _lp[0][2].sanitize();
    _lp[1][2].sanitize();
    _lp[0][3].sanitize();
    _lp[1][3].sanitize();
    _hp[0][0].sanitize();
    _hp[1][0].sanitize();
    _hp[0][1].sanitize();
    _hp[1][1].sanitize();
    _meter.Process(out_buffer);
//    for (int i=0;i<len;i++)
//    {
//        _buffer[i+fir_len] = in_buffer_set[0]->GetData()[i];
//    }

//    for (int i=0;i<bc->frames*bc->channels;i++)
//    {
//        out_buffer->GetData()[i] =0 ;
//        for (int j=0;j<fir_len;j++)
//        {
//            out_buffer->GetData()[i] += coeffs[j]*_buffer[fir_len + i - j ];
//        }

//        out_buffer->GetData()[i]=CLIP(tap.process(out_buffer->GetData()[i]));
//    }

//    for (int i=0;i<fir_len;i++)
//    {
//        _buffer[i]=in_buffer_set[0]->GetData()[bc->frames*bc->channels - fir_len +i];
//    }
*/
    return true;
}

void Vrok::EffectFIR::PropertyChanged(PropertyBase *property)
{
    if ( !(_blend.Get() < 10.0 && _blend.Get() > -10.0) )
    {
        WARN(9, "invalid value for blend");
        return;
    }

    if ( !(_drive.Get() < 10.0 && _drive.Get() > 0.0) )
    {
        WARN(9, "invalid value for drive");
        return;
    }
    if ( !(_hp_freq.Get() < 20000.0 && _hp_freq.Get() > 0.0) )
    {
        WARN(9, "invalid value for hp_freq");
        return;
    }
    if ( !(_lp_freq.Get() < 20000.0 && _lp_freq.Get() > 0.0) )
    {
        WARN(9, "invalid value for lp_freq");
        return;
    }

    if ( !(_dry_vol.Get() < 20.0 && _dry_vol.Get() > 0.0) )
    {
        WARN(9, "invalid value for dry vol");
        return;
    }

    if ( !(_wet_vol.Get() < 20.0 && _wet_vol.Get() > 0.0) )
    {
        WARN(9, "invalid value for wet vol");
        return;
    }

    _lp[0][0].set_lp_rbj(_lp_freq.Get(), 0.707, (float)GetBufferConfig()->samplerate);
    _lp[0][1].copy_coeffs(_lp[0][0]);
    _lp[0][2].copy_coeffs(_lp[0][0]);
    _lp[0][3].copy_coeffs(_lp[0][0]);
    _lp[1][0].copy_coeffs(_lp[0][0]);
    _lp[1][1].copy_coeffs(_lp[0][0]);
    _lp[1][2].copy_coeffs(_lp[0][0]);
    _lp[1][3].copy_coeffs(_lp[0][0]);


    _hp[0][0].set_hp_rbj(_hp_freq.Get(), 0.707, (float)GetBufferConfig()->samplerate);
    _hp[0][1].copy_coeffs(_hp[0][0]);
    _hp[1][0].copy_coeffs(_hp[0][0]);
    _hp[1][1].copy_coeffs(_hp[0][0]);

    _dist[0].set_sample_rate((float)GetBufferConfig()->samplerate);
    _dist[0].set_params(_blend.Get(),_drive.Get());
    _dist[1].set_sample_rate((float)GetBufferConfig()->samplerate);
    _dist[1].set_params(_blend.Get(),_drive.Get());

    _f32_dry_vol = _dry_vol.Get();
    _f32_wet_vol = _wet_vol.Get();


}

bool Vrok::EffectFIR::BufferConfigChange(BufferConfig *config)
{
    DBG(0, "bufferchange fot vu");
    _meter.SetBufferConfig(config);

    return true;
}
