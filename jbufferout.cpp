#include "jbufferout.h"

void vrok::DriverJBufferOut::SetEvents(vrok::DriverJBufferOut::Events *events) {
    m_events = events;
}

std::vector<vrok::Driver::DeviceInfo> vrok::DriverJBufferOut::GetDeviceInfo() {
    DeviceInfo devInfo;
    devInfo.name = "JNI";
    devInfo.user_data = nullptr;
    std::vector<DeviceInfo> devList;
    devList.push_back(devInfo);
    return devList;
}

std::string vrok::DriverJBufferOut::GetDefaultDevice() {
    return "JNI";
}

bool vrok::DriverJBufferOut::SetDevice(std::string device) {
    return true;
}

void vrok::DriverJBufferOut::ThreadStart() {
    m_events->OnThreadStart();
}

void vrok::DriverJBufferOut::ThreadEnd() {
    m_events->OnThreadEnd();
}

vrok::DriverJBufferOut::~DriverJBufferOut() { }

bool vrok::DriverJBufferOut::BufferConfigChange(BufferConfig *config) {
    m_events->OnBufferConfigChange(config->frames, config->samplerate, config->channels);
    int len = config->frames * config->channels;
    if (m_tempBuffer.size() < len)
        m_tempBuffer.resize(len);

    Sleep(10);

    return true;
}

bool vrok::DriverJBufferOut::DriverRun(Buffer *buffer) {
    if (buffer->GetBufferType() == Buffer::Type::StreamEnd) {
        DBG(0, "stream end received");
    }
    int len = buffer->GetBufferConfig()->frames * buffer->GetBufferConfig()->channels;
    for (int i = 0; i < len; i++) {
        m_tempBuffer[i] = double(buffer->GetData()[i]);
    }
    m_events->OnBuffer(m_tempBuffer.data(), buffer->GetBufferConfig()->frames, buffer->GetBufferType());
    return true;
}

void vrok::DriverJBufferOut::Events::OnThreadStart() { }

void vrok::DriverJBufferOut::Events::OnThreadEnd() { }
