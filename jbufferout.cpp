#include "jbufferout.h"

Vrok::DriverJBufferOut::DriverJBufferOut()
{

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

}

bool Vrok::DriverJBufferOut::DriverRun(Buffer *buffer)
{

}
