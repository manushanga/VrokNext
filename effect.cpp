#include "effect.h"
#include <cstring>
Vrok::Effect::Effect() :
    BufferGraph::Point(),
    _work(true)
{

}

void Vrok::Effect::Run()
{
    int len=GetBufferConfig()->channels * GetBufferConfig()->frames;

    auto buffers=PeakAllSources();
    if (buffers)
    {
        auto buffer=AcquireBuffer();
        if (buffer)
        {
            BufferConfig *c=buffers[0]->GetBufferConfig();

            if ((*c) != *buffer->GetBufferConfig())
            {
               // DBG("xx");
                SetBufferConfig(c);
                BufferConfigChange(c);
                //GetBufferConfig()->Print();
                buffer->Reset(c);
                len=c->channels * c->frames;
            }
            buffer->SetStreamId(buffers[0]->GetStreamId());
            memset(buffer->GetData(),0,len*sizeof(double));
            buffer->GetWatch() = buffers[0]->GetWatch();

            _work=EffectRun(buffer, buffers, _sources.size());

            PushBuffer(buffer);
        }

        ReleaseAllSources(buffers);
    }
}
