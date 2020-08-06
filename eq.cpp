#include <cmath>

#include "eq.h"

#define CLIP(__x) ((__x>1.0f)?1.0f:(__x<-1.0f?-1.0f:__x))
#define DB_TO_A(__db) (std::pow(10,(__db/-20.0)))
#define A_TO_DB(__a) (-20.0 * std::log(__a))
const char* g_eq_desc_temp =
        "cpu: %s\n"
        "fft hw accel: %s\n"
        "fft impl: %s\n";
#define DESC_BUF_LEN 64

Vrok::EffectSSEQ::EffectSSEQ() :
    _sb_paramsroot(nullptr)
{
    ComponentManager *c=ComponentManager::GetSingleton();

    c->RegisterComponent(this);
    std::vector<char> bandname;
    bandname.resize(16);
    for (int i=0;i<EQ_BAND_COUNT;i++)
    {
        snprintf(bandname.data(), 16, "Band%02d", i);
        c->RegisterProperty(this, bandname.data(), &_bands[i]);
    }

    memset(&_sb_state, 0, sizeof(SuperEqState));
    
    c->RegisterProperty(this,"Preamp",&_preamp);

    for (int i=0;i<EQ_BAND_COUNT;i++)
    {
        _bands[i].Set(1.0);
    }
    _preamp.Set(1.0);
    _desc_buffer.resize(DESC_BUF_LEN);
    _eq_amp = nullptr;
    _shm = sharedmem_create("vrok.eq.band.values", EQ_BAND_COUNT * sizeof(float));
    _eq_amp_shm = (float*)_shm->buffer;
}

bool Vrok::EffectSSEQ::EffectRun(Buffer *out_buffer, Buffer **in_buffer_set, int buffer_count)
{

    std::lock_guard<std::mutex> lg(_eq_setting_guard);

    BufferConfig *bc=in_buffer_set[0]->GetBufferConfig();
    int len=bc->frames*bc->channels;

    out_buffer->Reset(bc);
    real_t *proc=in_buffer_set[0]->GetData();
    real_t *proc_out=out_buffer->GetData();
    

    memcpy(proc_out, proc, sizeof(real_t) * len);

    equ_modifySamples_real<real_t>(&_sb_state, (char *)proc_out, bc->frames, bc->channels);

    for (int i=0;i<EQ_BAND_COUNT;i++)
    {
        _eq_amp_shm[i] = _eq_amp[i];
    }

    return true;
}

void Vrok::EffectSSEQ::PropertyChanged(PropertyBase *property)
{
    std::lock_guard<std::mutex> lg(_eq_setting_guard);
    BufferConfig *bc=GetBufferConfig();
    void *params = paramlist_alloc ();
    float sb_bands_copy[EQ_BAND_COUNT];
    
    for (int i=0;i<EQ_BAND_COUNT;i++){
        sb_bands_copy[i]=DB_TO_A(_bands[i].Get())* DB_TO_A(_preamp.Get());
    }

    DBG(0, "eq sr "<<bc->samplerate)

    equ_makeTable (&_sb_state, sb_bands_copy, params, bc->samplerate);
    if (_sb_paramsroot)
        paramlist_free (_sb_paramsroot);
    _sb_paramsroot = params;


}

bool Vrok::EffectSSEQ::BufferConfigChange(BufferConfig *config)
{
    std::lock_guard<std::mutex> lg(_eq_setting_guard);

    DBG(0, "000000000oo");
    //*config->Print();
    void *params = paramlist_alloc ();
    float sb_bands_copy[EQ_BAND_COUNT];

    for (int i=0;i<EQ_BAND_COUNT;i++){
        sb_bands_copy[i]=DB_TO_A(_bands[i].Get())* DB_TO_A(_preamp.Get());
    }

    //equ_quit();
    equ_clearbuf(&_sb_state);
    equ_init (&_sb_state, 12, config->channels);
    _eq_amp = equ_band_amplitudes(&_sb_state);

        DBG(0, "eq sr "<<config->samplerate)

    equ_makeTable (&_sb_state, sb_bands_copy, params, config->samplerate);
    if (_sb_paramsroot)
        paramlist_free (_sb_paramsroot);
    _sb_paramsroot = params;

    return true;
}

Vrok::EffectSSEQ::~EffectSSEQ()
{
    equ_quit(&_sb_state);
    sharedmem_destroy(_shm);
}

const char *Vrok::EffectSSEQ::Description()
{
    std::string cpu;
#if defined(__arm__)
    cpu = "arm";
#elif defined(__aarch64__)
    cpu = "aarch64";
#elif defined(__x86_64__)
    cpu = "x86_64";
#else
    cpu = "generic";
#endif

    snprintf(_desc_buffer.data(), DESC_BUF_LEN, g_eq_desc_temp,
             cpu.c_str(),equ_fftAccel(&_sb_state),equ_fftImpl(&_sb_state) );

    return _desc_buffer.data();
}
