#include "driver.h"

Vrok::Driver::Driver() :
    _work(true)
{

}

void Vrok::Driver::Run()
{
    auto buffers=PeakAllSources();
    if (buffers)
    {
        BufferConfig *c=buffers[0]->GetBufferConfig();
        //c->Print();
        //GetBufferConfig()->Print();
        if (*c!= *GetBufferConfig())
        {
            BufferConfigChange(c);
            SetBufferConfig(c);
        }
        // unchecked mixing
        for (int i=0;i<_sources.size();i++)
        {
            for (int j=0;j<c->frames * c->channels;j++)
            {
                buffers[0]->GetData()[j]=0.5*buffers[0]->GetData()[j]+0.5*buffers[i]->GetData()[j];
            }
        }
        
        _work=DriverRun(buffers[0]);
        ReleaseAllSources(buffers);
    }
}
