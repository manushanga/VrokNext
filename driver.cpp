#include "driver.h"

Vrok::Driver::Driver() :
    _work(true)
{

}

void Vrok::Driver::Run()
{
    while (_work)
    {

        auto buffers=PeakAllSources();

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