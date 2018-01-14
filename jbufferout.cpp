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
    m_events->OnThreadStart();
}

void Vrok::DriverJBufferOut::ThreadEnd()
{
    m_events->OnThreadEnd();
}

Vrok::DriverJBufferOut::~DriverJBufferOut()
{

}

bool Vrok::DriverJBufferOut::BufferConfigChange(BufferConfig *config)
{
    m_events->OnBufferConfigChange(config->frames, config->samplerate, config->channels);
    int len = config->frames * config->channels;
    if (m_tempBuffer.size() < len)
        m_tempBuffer.resize(len);

    Sleep(10);

    return true;
}

bool Vrok::DriverJBufferOut::DriverRun(Buffer *buffer)
{
    if (buffer->getBufferType() == Buffer::Type::StreamEnd)
    {
        DBG(0,"stream end received");
    }
    int len = buffer->GetBufferConfig()->frames * buffer->GetBufferConfig()->channels;
    for (int i=0;i<len;i++)
    {
        m_tempBuffer[i] = double( buffer->GetData()[i] );
    }
    m_events->OnBuffer(m_tempBuffer.data(), buffer->GetBufferConfig()->frames, buffer->getBufferType());
    return true;
}

void Vrok::DriverJBufferOut::Events::OnThreadStart()
{

}

void Vrok::DriverJBufferOut::Events::OnThreadEnd()
{

}
