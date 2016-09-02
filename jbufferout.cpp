#include <ao/ao.h>
#include "jbufferout.h"

ao_device *_ao_device=nullptr;

void Vrok::DriverJBufferOut::SetEvents(Vrok::DriverJBufferOut::Events *events)
{
    m_events = events;
}

Vrok::DriverJBufferOut::DriverJBufferOut() :
    _out_sr(-1)
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
    if (_out_sr == -1)
        _sr_data.src_ratio = 1;
    else
        _sr_data.src_ratio = double(_out_sr)/ double(config->samplerate);

    int err=0;
    _current_state = src_new(SRC_SINC_FASTEST, config->channels, &err);

    if (_ao_device)
    {
        ao_close(_ao_device);
        ao_shutdown();
    }
    ao_initialize();

    ao_sample_format sformat;

    sformat.channels=config->channels;
    if (_out_sr == -1)
        sformat.rate=config->samplerate;
    else
        sformat.rate=_out_sr;
    sformat.bits=16;
    sformat.byte_format=AO_FMT_NATIVE;
    sformat.matrix=0;

    _ao_device=ao_open_live(ao_default_driver_id(),&sformat,NULL);

    return true;
}

bool Vrok::DriverJBufferOut::DriverRun(Buffer *buffer)
{

    SRC_DATA data_copy = _sr_data;

    assert(INTERNAL_BUFFER_SIZE >= buffer->GetBufferConfig()->channels * buffer->GetBufferConfig()->frames);
    int len = buffer->GetBufferConfig()->channels * buffer->GetBufferConfig()->frames;
    for (int i=0;i<len;i++)
    {
        _buffer[i] = float(buffer->GetData()[i]);
    }

    data_copy.data_in = &_buffer[0];
    data_copy.data_out = &_out_buffer[0];

    int out_frames=0;

    data_copy.end_of_input = 0;
    data_copy.output_frames = buffer->GetBufferConfig()->frames * _sr_data.src_ratio * 2;
    data_copy.output_frames_gen = 1;
    out_frames=0;

    data_copy.input_frames = buffer->GetBufferConfig()->frames ;

    while (data_copy.output_frames_gen > 0) {
        src_process(_current_state,&data_copy);

        data_copy.input_frames -= data_copy.input_frames_used;
        data_copy.data_in += data_copy.input_frames_used* buffer->GetBufferConfig()->channels;
        out_frames+=data_copy.output_frames_gen;
        //std::cout<< out_frames << data_copy.output_frames  <<std::endl;
    }

    assert(out_frames < INTERNAL_BUFFER_SIZE);

    for (int i=0;i<out_frames* buffer->GetBufferConfig()->channels;i++)
    {
        _out_dbl_buffer[i] = double(_out_buffer[i]);
    }

    if (_out_sr == -1)
        m_events->OnBufferConfigChange(buffer->GetBufferConfig()->frames, buffer->GetBufferConfig()->samplerate ,buffer->GetBufferConfig()->channels);
    else
        m_events->OnBufferConfigChange(out_frames, _out_sr, buffer->GetBufferConfig()->channels);


    m_events->OnBuffer(&_out_dbl_buffer[0]);

    uint16_t cbuf[8192*4];

    //int samples=GetBufferConfig()->channels * GetBufferConfig()->frames;
    int samples=out_frames * GetBufferConfig()->channels;

    assert(8192*4 > samples);
    for (int i=0;i<samples;i++)
    {
        //cbuf[i]=(uint16_t)(buffer->GetData()[i]*32767.0);
        cbuf[i]=(uint16_t)(_out_dbl_buffer[i]*32767.0);

    }

    ao_play(_ao_device,(char *) &cbuf[0],samples*sizeof(uint16_t));

    return true;
}

void Vrok::DriverJBufferOut::SetOutputSamplerate(int samplerate)
{
    _out_sr = samplerate;
}
