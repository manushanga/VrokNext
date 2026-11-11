#include "audioout.h"
#include <algorithm>
#include <cstdint>
#include <mutex>
#include <vector>

using namespace std;

namespace {
std::mutex &aoMutex() { static std::mutex mutex; return mutex; }
unsigned &aoUsers() { static unsigned users = 0; return users; }
}


vrok::DriverAudioOut::DriverAudioOut() : _ao_device(nullptr), _volume(0), _new_resource(false), _device_id(-1) {
    std::lock_guard<std::mutex> lock(aoMutex());
    if (aoUsers()++ == 0) ao_initialize();
    SetDevice("");
}

vrok::DriverAudioOut::~DriverAudioOut() {
    std::lock_guard<std::mutex> lock(aoMutex());
    if (_ao_device) ao_close(_ao_device);
    if (--aoUsers() == 0) ao_shutdown();
}

std::string vrok::DriverAudioOut::GetDefaultDevice() {
#ifdef __APPLE__
    int def = ao_driver_id("macosx");
#else
    int def = ao_default_driver_id();
#endif
    ao_info *info = ao_driver_info(def);
    return info ? std::string(info->short_name) : std::string();
}

bool vrok::DriverAudioOut::SetDevice(string device) {
    if (device == "") {
        device = GetDefaultDevice();
    }
    auto vec = GetDeviceInfo();
    for (int i = 0; i < vec.size(); i++) {
        if (vec[i].name == device) {
            int id = ao_driver_id(vec[i].name.c_str());
            if (id > -1) {
                _device_id = id;
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

std::vector<vrok::Driver::DeviceInfo> vrok::DriverAudioOut::GetDeviceInfo() {
    std::vector<DeviceInfo> interface_info;
    int count = 0;
    ao_info **info = ao_driver_info_list(&count);
    for (int i = 0; i < count; i++) {
        DeviceInfo int_info;
        int_info.name = std::string(info[i]->short_name);
        int_info.user_data = (void *)info[i];
        interface_info.push_back(int_info);
    }
    return interface_info;
}
bool vrok::DriverAudioOut::BufferConfigChange(BufferConfig *config) {
    if (!_ao_device || *GetBufferConfig() != *config) {
        if (_ao_device) {
            ao_close(_ao_device);
            _ao_device = nullptr;
        }
        ao_sample_format sformat;

        sformat.channels = config->channels;
        sformat.rate = config->samplerate;
        sformat.bits = 16;
        sformat.byte_format = AO_FMT_NATIVE;
        sformat.matrix = 0;

        _ao_device = ao_open_live(_device_id, &sformat, NULL);

        if (!_ao_device) return false;

        DBG(1, "o " << config->channels);

        DBG(1, "o " << config->samplerate);
    }
    return true;
}
bool vrok::DriverAudioOut::DriverRun(Buffer *buffer) {
    // DBG(buffer);
    if (!_ao_device) return false;
    const size_t samples = GetBufferConfig()->channels * GetBufferConfig()->frames;
    std::vector<int16_t> cbuf(samples);
    for (size_t i = 0; i < samples; i++) {
        const float value = std::max(-1.0f, std::min(1.0f, buffer->GetData()[i]));
        cbuf[i] = static_cast<int16_t>(value * 32767.0f);
    }
    return ao_play(_ao_device, reinterpret_cast<char *>(cbuf.data()),
                   cbuf.size() * sizeof(int16_t)) != 0;

}

void vrok::DriverAudioOut::SetVolume(double volume) { }
