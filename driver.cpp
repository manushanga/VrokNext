#include "driver.h"

#define DB_TO_A(__db) (std::pow(2,(__db/10.0)))

Vrok::Driver::Driver() :
    BufferGraph::Point(),
    _work(true),
    _volume(1.0),
    _meter("out"),
    _input_bc(0,0,0),
    _first_run(true)
{

}

void Vrok::Driver::SetVolume(real_t volume)
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

        if ( _first_run || (_input_bc != *c))
        {
            if (BufferConfigChange(c) == false)
            {
                WARN(0, "BufferConfig failed");
                return ;
            }
            _first_run = false;
            _input_bc = *c;
        }
        // unchecked mixing
        for (size_t i=0;i<_sources.size();i++)
        {
            for (int j=0;j<c->frames * c->channels;j++)
            {
                real_t val = 0.5*buffers[0]->GetData()[j]+0.5*buffers[i]->GetData()[j];
                buffers[0]->GetData()[j]=val*_volume;
            }
        }
        _meter.Process(buffers[0]);
        _work=DriverRun(buffers[0]);
        ReleaseAllSources(buffers);
    }
    else
    {
        DBG(0, "output drv sleep");
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}
