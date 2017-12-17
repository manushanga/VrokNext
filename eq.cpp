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
    std::vector<char> bandname;
    bandname.resize(16);
    for (int i=0;i<BAND_COUNT;i++)
    {
        snprintf(bandname.data(), 16, "Band%02d", i);
        c->RegisterProperty(this, bandname.data(), &_bands[i]);
    }

    memset(&_sb_state, 0, sizeof(SuperEqState));
    
    c->RegisterProperty(this,"Preamp",&_preamp);

    for (int i=0;i<BAND_COUNT;i++)
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

    
    real_t *proc=in_buffer_set[0]->GetData();
    real_t *proc_out=out_buffer->GetData();
    
    memcpy(proc_out, proc, sizeof(real_t) * len);
    equ_modifySamples_real<real_t>(&_sb_state, (char *)proc_out, bc->frames, bc->channels);

    return true;
}

void Vrok::EffectSSEQ::PropertyChanged(PropertyBase *property)
{
    BufferConfig *bc=GetBufferConfig();
    void *params = paramlist_alloc ();
    float sb_bands_copy[BAND_COUNT];
    
    for (int i=0;i<BAND_COUNT;i++){
        sb_bands_copy[i]=DB_TO_A(_bands[i].Get())* DB_TO_A(_preamp.Get());
    }

    equ_makeTable (&_sb_state, sb_bands_copy, params, bc->samplerate);
    if (_sb_paramsroot)
        paramlist_free (_sb_paramsroot);
    _sb_paramsroot = params;

}

bool Vrok::EffectSSEQ::BufferConfigChange(BufferConfig *config)
{
    DBG(0, "000000000oo");
    //*config->Print();
    void *params = paramlist_alloc ();
    float sb_bands_copy[BAND_COUNT];

    for (int i=0;i<BAND_COUNT;i++){
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
