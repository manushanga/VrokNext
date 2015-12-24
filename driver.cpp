#include "driver.h"

#define DB_TO_A(__db) (std::pow(2,(__db/10.0)))

Vrok::Driver::Driver() :
    BufferGraph::Point(),
    _work(true),
    _volume(1.0),
    _meter("out")
{

}

void Vrok::Driver::SetVolume(double volume)
{
    _volume = DB_TO_A(volume);
}

std::vector<Vrok::VUMeter *> Vrok::Driver::GetMeters()
{
    std::vector<Vrok::VUMeter *> meters;
    meters.push_back(&_meter);
    return meters;
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
            std::cout<<"---channging conf"<<std::endl;
            BufferConfigChange(c);
            SetBufferConfig(c);
        }
        // unchecked mixing
        for (size_t i=0;i<_sources.size();i++)
        {
            for (int j=0;j<c->frames * c->channels;j++)
            {
                double val = 0.5*buffers[0]->GetData()[j]+0.5*buffers[i]->GetData()[j];
                buffers[0]->GetData()[j]=val*_volume;
            }
        }
        _meter.Process(buffers[0]);
        _work=DriverRun(buffers[0]);
        ReleaseAllSources(buffers);
    }
}
