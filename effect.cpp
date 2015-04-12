#include "effect.h"
#include <cstring>
Vrok::Effect::Effect() :
    _work(true)
{

}

void Vrok::Effect::Run()
{
    int len=GetBufferConfig()->channels * GetBufferConfig()->frames;

    auto buffer=AcquireBuffer();
    auto buffers=PeakAllSources();
    if (buffers)
    {
        BufferConfig *c=buffers[0]->GetBufferConfig();

        if ((*c) != *buffer->GetBufferConfig())
        {
           // DBG("xx");
            SetBufferConfig(c);

            //GetBufferConfig()->Print();
            buffer->Reset(c);
            len=c->channels * c->frames;
        }
        memset(buffer->GetData(),0,len*sizeof(float));

        _work=EffectRun(buffer, buffers, _sources.size());

        ReleaseAllSources(buffers);
        PushBuffer(buffer);
    }
}
