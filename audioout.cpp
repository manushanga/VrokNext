#include "audioout.h"

using namespace std;

vrok::DriverAudioOut::DriverAudioOut() : _ao_device(nullptr), _volume(0), _new_resource(false) {
    _device_id = ao_default_driver_id();
}

std::string vrok::DriverAudioOut::GetDefaultDevice() {
    int def = ao_default_driver_id();
    return std::string(ao_driver_info(def)->short_name);
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
    if (*GetBufferConfig() != *config) {
        if (_ao_device) {
            ao_close(_ao_device);
            ao_shutdown();
            ao_initialize();
        }
        ao_sample_format sformat;

        sformat.channels = config->channels;
        sformat.rate = config->samplerate;
        sformat.bits = 16;
        sformat.byte_format = AO_FMT_NATIVE;
        sformat.matrix = 0;

        _ao_device = ao_open_live(ao_default_driver_id(), &sformat, NULL);

        DBG(1, "o " << config->channels);

        DBG(1, "o " << config->samplerate);
    }
    return true;
}
bool vrok::DriverAudioOut::DriverRun(Buffer *buffer) {
    // DBG(buffer);
    DBG(6, "arrival: " << buffer->GetWatch().Stop());

    uint16_t cbuf[8192 * 4];

    int samples = GetBufferConfig()->channels * GetBufferConfig()->frames;

    for (int i = 0; i < samples; i++) {
        cbuf[i] = (uint16_t)(buffer->GetData()[i] * 32767.0);
    }

    ao_play(_ao_device, (char *)&cbuf[0], samples * sizeof(uint16_t));

    return true;
}

void vrok::DriverAudioOut::SetVolume(double volume) { }
