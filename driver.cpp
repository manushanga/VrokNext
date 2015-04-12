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
        _work=DriverRun(buffers[0]);
        ReleaseAllSources(buffers);
    }
}
