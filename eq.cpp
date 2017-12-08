#include <cmath>

#include "eq.h"

#define CLIP(__x) ((__x>1.0f)?1.0f:(__x<-1.0f?-1.0f:__x))
#define DB_TO_A(__db) (std::pow(2,(__db/10.0)))
#define A_TO_DB(__a) (10.0 * std::log(__a))

Vrok::EffectSSEQ::EffectSSEQ() :
    _sb_paramsroot(nullptr)
{
    
    ComponentManager *c=ComponentManager::GetSingleton();

    c->RegisterComponent(this);
    c->RegisterProperty(this,"band_0",&_bands[0]);
    c->RegisterProperty(this,"band_1",&_bands[1]);
    c->RegisterProperty(this,"band_2",&_bands[2]);
    c->RegisterProperty(this,"band_3",&_bands[3]);
    c->RegisterProperty(this,"band_4",&_bands[4]);
    c->RegisterProperty(this,"band_5",&_bands[5]);
    c->RegisterProperty(this,"band_6",&_bands[6]);
    c->RegisterProperty(this,"band_7",&_bands[7]);
    c->RegisterProperty(this,"band_8",&_bands[8]);
    c->RegisterProperty(this,"band_9",&_bands[9]);
    c->RegisterProperty(this,"band_10",&_bands[10]);
    c->RegisterProperty(this,"band_11",&_bands[11]);
    c->RegisterProperty(this,"band_12",&_bands[12]);
    c->RegisterProperty(this,"band_13",&_bands[13]);
    c->RegisterProperty(this,"band_14",&_bands[14]);
    c->RegisterProperty(this,"band_15",&_bands[15]);
    c->RegisterProperty(this,"band_16",&_bands[16]);
    c->RegisterProperty(this,"band_17",&_bands[17]);
    
    memset(&_sb_state, 0, sizeof(SuperEqState));
    
    c->RegisterProperty(this,"preamp",&_preamp);

    for (int i=0;i<BAR_COUNT;i++)
    {
        _bands[i].Set(0.0);
    }
    _preamp.Set(0.0);

    //BufferConfigChange(GetBufferConfig());
}

bool Vrok::EffectSSEQ::EffectRun(Buffer *out_buffer, Buffer **in_buffer_set, int buffer_count)
{
    BufferConfig *bc=GetBufferConfig();
    int len=bc->frames*bc->channels;

    
    double *proc=in_buffer_set[0]->GetData();
    double *proc_out=out_buffer->GetData();
    
    memcpy(proc_out, proc, sizeof(double) * len);
    equ_modifySamples_double(&_sb_state, (char *)proc_out, bc->frames, bc->channels);

    return true;
}

void Vrok::EffectSSEQ::PropertyChanged(PropertyBase *property)
{
    BufferConfig *bc=GetBufferConfig();
    void *params = paramlist_alloc ();
    float sb_bands_copy[BAR_COUNT];
    
    for (int i=0;i<BAR_COUNT;i++){
        sb_bands_copy[i]=DB_TO_A(_bands[i].Get())* DB_TO_A(_preamp.Get());
    }

    equ_makeTable (&_sb_state, sb_bands_copy, params, bc->samplerate);
    if (_sb_paramsroot)
        paramlist_free (_sb_paramsroot);
    _sb_paramsroot = params;

}

bool Vrok::EffectSSEQ::BufferConfigChange(BufferConfig *config)
{
    DBG(0,"000000000oo");
    //*config->Print();
    void *params = paramlist_alloc ();
    float sb_bands_copy[BAR_COUNT];

    for (int i=0;i<BAR_COUNT;i++){
        sb_bands_copy[i]=DB_TO_A(_bands[i].Get())* DB_TO_A(_preamp.Get());
    }

    //equ_quit();
    //equ_clearbuf()
    equ_init (&_sb_state, 14, config->channels);

    equ_makeTable (&_sb_state, sb_bands_copy, params, config->samplerate);
    if (_sb_paramsroot)
        paramlist_free (_sb_paramsroot);
    _sb_paramsroot = params;

    return true;
}
