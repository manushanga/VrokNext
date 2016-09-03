#include "effect.h"
#include <cstring>
Vrok::Effect::Effect() :
    BufferGraph::Point(),
    _input_bc(0,0,0),
    _first_run(true),
    _work(true)
{

}

void Vrok::Effect::Run()
{

    auto buffers=PeakAllSources();
    if (buffers)
    {
        auto buffer=AcquireBuffer();
        if (buffer)
        {
            BufferConfig *c=buffers[0]->GetBufferConfig();

            if ( _first_run || (_input_bc != *c))
            {
                if (BufferConfigChange(c) == false)
                {
                    WARN(0,"BufferConfig failed");
                    return ;
                }
                _first_run = false;
                _input_bc = *c;
            }

            buffer->SetStreamId(buffers[0]->GetStreamId());

            buffer->GetWatch() = buffers[0]->GetWatch();

            _work=EffectRun(buffer, buffers, _sources.size());

            PushBuffer(buffer);
        }

        ReleaseAllSources(buffers);
    }
}
