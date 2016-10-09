#include "jbufferout.h"

void Vrok::DriverJBufferOut::SetEvents(Vrok::DriverJBufferOut::Events *events)
{
    m_events = events;
}

std::vector<Vrok::Driver::DeviceInfo> Vrok::DriverJBufferOut::GetDeviceInfo()
{
    DeviceInfo devInfo;
    devInfo.name = "JNI";
    devInfo.user_data = nullptr;
    std::vector<DeviceInfo> devList;
    devList.push_back(devInfo);
    return devList;
}

std::string Vrok::DriverJBufferOut::GetDefaultDevice()
{
    return "JNI";
}

bool Vrok::DriverJBufferOut::SetDevice(std::string device)
{
    return true;
}

void Vrok::DriverJBufferOut::ThreadStart()
{

}

void Vrok::DriverJBufferOut::ThreadEnd()
{

}

Vrok::DriverJBufferOut::~DriverJBufferOut()
{

}

bool Vrok::DriverJBufferOut::BufferConfigChange(BufferConfig *config)
{
    m_events->OnBufferConfigChange(config->frames, config->samplerate, config->channels);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    return true;
}

bool Vrok::DriverJBufferOut::DriverRun(Buffer *buffer)
{
    m_events->OnBuffer(buffer->GetData());
    return true;
}
