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
#define CLIP(x) ((x>1.0f)?1.0f:(x<-1.0f?-1.0f:x))
float coeffs[] = {

    0.001455,
    0.001472,
    0.001523,
    0.001607,
    0.001723,
    0.001871,
    0.002051,
    0.002263,
    0.002504,
    0.002775,
    0.003074,
    0.003401,
    0.003753,
    0.004131,
    0.004531,
    0.004953,
    0.005395,
    0.005855,
    0.006332,
    0.006824,
    0.007328,
    0.007843,
    0.008366,
    0.008896,
    0.009431,
    0.009968,
    0.010505,
    0.011040,
    0.011572,
    0.012097,
    0.012614,
    0.013121,
    0.013616,
    0.014096,
    0.014560,
    0.015006,
    0.015433,
    0.015837,
    0.016219,
    0.016576,
    0.016907,
    0.017211,
    0.017486,
    0.017732,
    0.017946,
    0.018129,
    0.018280,
    0.018398,
    0.018483,
    0.018534,
    0.018551,
    0.018534,
    0.018483,
    0.018398,
    0.018280,
    0.018129,
    0.017946,
    0.017732,
    0.017486,
    0.017211,
    0.016907,
    0.016576,
    0.016219,
    0.015837,
    0.015433,
    0.015006,
    0.014560,
    0.014096,
    0.013616,
    0.013121,
    0.012614,
    0.012097,
    0.011572,
    0.011040,
    0.010505,
    0.009968,
    0.009431,
    0.008896,
    0.008366,
    0.007843,
    0.007328,
    0.006824,
    0.006332,
    0.005855,
    0.005395,
    0.004953,
    0.004531,
    0.004131,
    0.003753,
    0.003401,
    0.003074,
    0.002775,
    0.002504,
    0.002263,
    0.002051,
    0.001871,
    0.001723,
    0.001607,
    0.001523,
    0.001472,
    0.001455
};


Vrok::EffectFIR::EffectFIR() :
    _buffer(new float[(FIR_LEN+GetBufferConfig()->frames)* GetBufferConfig()->channels])
{
    int len=(FIR_LEN+GetBufferConfig()->frames)* GetBufferConfig()->channels;
    for (int i=0;i<len;i++)
    {
        _buffer[i] = 0;
    }
    dist[0].activate();
    dist[1].activate();
    ComponentManager *c=ComponentManager::GetSingleton();

    c->RegisterComponent(this);
    c->RegisterProperty(this,"blend",&blend);
    c->RegisterProperty(this,"drive",&drive);

    c->RegisterProperty(this,"lp_freq",&lp_freq);
    c->RegisterProperty(this,"hp_freq",&hp_freq);
    c->RegisterProperty(this,"dry_vol",&dry_vol);
    c->RegisterProperty(this,"wet_vol",&wet_vol);

    blend.Set(7);
    drive.Set(7);

    lp_freq.Set(80.0f);
    hp_freq.Set(40.0f);

    wet_vol.Set(0.7);
    dry_vol.Set(0.3);

    _dry_vol = dry_vol.Get();
    _wet_vol = wet_vol.Get();

    lp[0][0].set_lp_rbj(lp_freq.Get(), 0.707, (float)48000.0);
    lp[0][1].copy_coeffs(lp[0][0]);
    lp[0][2].copy_coeffs(lp[0][0]);
    lp[0][3].copy_coeffs(lp[0][0]);
    lp[1][0].copy_coeffs(lp[0][0]);
    lp[1][1].copy_coeffs(lp[0][0]);
    lp[1][2].copy_coeffs(lp[0][0]);
    lp[1][3].copy_coeffs(lp[0][0]);
    hp[0][0].set_hp_rbj(hp_freq.Get(), 0.707, (float)48000.0);
    hp[0][1].copy_coeffs(hp[0][0]);
    hp[1][0].copy_coeffs(hp[0][0]);
    hp[1][1].copy_coeffs(hp[0][0]);
    dist[0].set_sample_rate(48000.0);
    dist[0].set_params(blend.Get(),drive.Get());
    dist[1].set_sample_rate(48000.0);
    dist[1].set_params(blend.Get(),drive.Get());


}

bool Vrok::EffectFIR::EffectRun(Buffer *out_buffer, Buffer **in_buffer_set, int buffer_count)
{
    BufferConfig *bc=GetBufferConfig();
    int fir_len=FIR_LEN*bc->channels;
    int len=bc->frames*bc->channels;

    /*for (int i=0;i<len;i++)
    {
        out_buffer->GetData()[i] = CLIP(tap.process(in_buffer_set[0]->GetData()[i]));
    }*/
    float *proc=in_buffer_set[0]->GetData();
    float *proc_out=out_buffer->GetData();
    for (int j=0;j<bc->frames;j++) {

        for (int i=0;i<bc->channels;i++)
        {
            proc_out[i] = lp[i][1].process(lp[i][0].process(proc[i]));

            proc_out[i] = dist[i].process(proc_out[i]);

            proc_out[i] = hp[i][0].process(hp[i][1].process(proc_out[i]));

            proc_out[i] = CLIP((proc_out[i]*_wet_vol + _dry_vol*proc[i])*1.5);
        }
        proc+=bc->channels;
        proc_out+=bc->channels;
    }
    lp[0][0].sanitize();
    lp[1][0].sanitize();
    lp[0][1].sanitize();
    lp[1][1].sanitize();
    lp[0][2].sanitize();
    lp[1][2].sanitize();
    lp[0][3].sanitize();
    lp[1][3].sanitize();
    hp[0][0].sanitize();
    hp[1][0].sanitize();
    hp[0][1].sanitize();
    hp[1][1].sanitize();
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

    return true;
}

void Vrok::EffectFIR::PropertyChanged(PropertyBase *property)
{
    DBG("lp"<<lp_freq.Get());

    lp[0][0].set_lp_rbj(lp_freq.Get(), 0.707, (float)48000.0);
    lp[0][1].copy_coeffs(lp[0][0]);
    lp[0][2].copy_coeffs(lp[0][0]);
    lp[0][3].copy_coeffs(lp[0][0]);
    lp[1][0].copy_coeffs(lp[0][0]);
    lp[1][1].copy_coeffs(lp[0][0]);
    lp[1][2].copy_coeffs(lp[0][0]);
    lp[1][3].copy_coeffs(lp[0][0]);


    hp[0][0].set_hp_rbj(hp_freq.Get(), 0.707, (float)48000.0);
    hp[0][1].copy_coeffs(hp[0][0]);
    hp[1][0].copy_coeffs(hp[0][0]);
    hp[1][1].copy_coeffs(hp[0][0]);

    dist[0].set_sample_rate(48000.0);
    dist[0].set_params(blend.Get(),drive.Get());
    dist[1].set_sample_rate(48000.0);
    dist[1].set_params(blend.Get(),drive.Get());

    _dry_vol = dry_vol.Get();
    _wet_vol = wet_vol.Get();


}
