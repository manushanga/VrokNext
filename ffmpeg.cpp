/*
  Vrok - smokin' audio
  (C) 2013 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.

  Based on,
  http://stackoverflow.com/questions/9799560/decode-audio-using-libavcodec-and-play-using-libao
  http://dranger.com/ffmpeg/tutorial05.html
  http://dranger.com/ffmpeg/tutorial04.html

*/
#include <limits>
#include "ffmpeg.h"

#define DIE_ON_ERR(ret) \
    if (ret < 0) \
        return false;
#define SHORTTOFL (1.0f/32768.0f)
#define SEEK_MAX 0xFFFFFFFFFFFFFFFFL

Vrok::DecoderFFMPEG::DecoderFFMPEG() :
    ctx(nullptr),
    container(nullptr)
{
    static long s=0;
    if (s==0) {
        avformat_network_init();
        av_register_all();
    }
    s++;
    container=avformat_alloc_context();
    _ringbuffer = new Ringbuffer<double>(
                2*FFMPEG_MAX_BUF_SIZE +
                2*FF_INPUT_BUFFER_PADDING_SIZE);
    //container->interrupt_callback.callback = FFMPEGDecoder::ff_avio_interrupt;
    //container->interrupt_callback.opaque = this;
    //last_read = time(NULL);
/*
    ao_initialize();
    ao_sample_format sformat;

    sformat.channels=2;
    sformat.rate=44100;
    sformat.bits=16;
    sformat.byte_format=AO_FMT_NATIVE;
    sformat.matrix=0;

    _ao_device=ao_open_live(ao_default_driver_id(),&sformat,NULL);
*/
}

Vrok::DecoderFFMPEG::~DecoderFFMPEG()
{
    Close();
}

bool Vrok::DecoderFFMPEG::Open(Vrok::Resource *resource)
{
    container=nullptr;
    ctx=nullptr;
    _ringbuffer->Clear();

    audio_stream_id = -1;
    if(avformat_open_input(&container,resource->_filename.c_str(),NULL,NULL)<0){
        WARN(9,"Can't open file");
        Close();
        return false;
    }

    if(avformat_find_stream_info(container, NULL)<0){
        WARN(9,"Stream info load failed");
        return false;
    }

    unsigned int i;
    for(i=0;i<container->nb_streams;i++){
        if(container->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
            audio_stream_id=i;
            break;
        }
    }
    if(audio_stream_id==-1){
        WARN(9,"No audio stream");
        Close();
        return false;
    }

    audio_st = container->streams[audio_stream_id];
    ctx=container->streams[audio_stream_id]->codec;
    codec=avcodec_find_decoder(ctx->codec_id);

    DBG(1,"codec: "<<codec->long_name);
    if(codec==NULL){
        WARN(9,"Cannot find codec");
        Close();
        return false;
    }

    if(avcodec_open2(ctx,codec,NULL)<0){
        WARN(9,"Codec cannot be opened");
        Close();
        return false;
    }

    sfmt=ctx->sample_fmt;


    duration_in_seconds = container->duration / AV_TIME_BASE;

    frameFinished=0;
    packetFinished=0;
    vpbuffer_write=0;
    temp_write=0;
    remainder_read=0;
    remainder_counter=0;

    av_init_packet(&packet);
    frame=av_frame_alloc();

    current_in_seconds=0;
    DBG(1,"opend");

    return true;
}

bool Vrok::DecoderFFMPEG::GetBufferConfig(BufferConfig *config)
{
    config->channels = ctx->channels;
    config->samplerate=ctx->sample_rate;
    DBG(1,"p "<<config->channels);

    DBG(1,"p "<<config->samplerate);

    return true;
}

bool Vrok::DecoderFFMPEG::Close()
{
    if (ctx)
        avcodec_close(ctx);
    ctx = nullptr;

    if (container) {
        avformat_close_input(&container);
        avformat_free_context(container);
    }
    container = nullptr;

    return true;
}
/* Notes:
 *
 * Everyone forgets, the original author does too. So some notes about the
 * routine follows,
 *
 * As every call should guarantee a filled buffer we first try to empty the
 * ringbuffer, if the ringbuffer seems to be empty it'll be filled and a buffer
 * will be taken from it.
 *
 * FFmpeg has audio and video packets and might even happen to have more types
 * of packets we are only interested in audio packets for now, therefore we
 * eat up other types of packets and only decode audio.
 *
*/
bool Vrok::DecoderFFMPEG::DecoderRun(Buffer *buffer,  BufferConfig *config)
{
    packetFinished = 0;
    while (!_ringbuffer->Read(buffer->GetData(),config->channels*config->frames))
    {
        packetFinished = av_read_frame(container,&packet);

        // eat up video packets!
        while (packet.stream_index!=audio_stream_id && packetFinished >=0)
        {
            av_free_packet(&packet);
            packetFinished = av_read_frame(container,&packet);
        }

        DIE_ON_ERR(packetFinished);

        int ret=0;
        // process audio packets
        ret = avcodec_decode_audio4(ctx,frame,&frameFinished,&packet);

        DIE_ON_ERR(ret);

        ret = av_samples_get_buffer_size(&plane_size, ctx->channels,
                                            frame->nb_samples,
                                            ctx->sample_fmt, 1);
        DIE_ON_ERR(ret);

        temp_write=0;

        if(frameFinished){
            current_in_seconds = ( audio_st->time_base.num * frame->pkt_pts )/ audio_st->time_base.den ;
            switch (sfmt){

                case AV_SAMPLE_FMT_S16P:

                    for (size_t nb=0;nb<plane_size/sizeof(uint16_t);nb++){
                        for (int ch = 0; ch < ctx->channels; ch++) {
                            temp[temp_write] = ((short *) frame->extended_data[ch])[nb] * SHORTTOFL;
                            temp_write++;
                        }
                    }
                    break;
                case AV_SAMPLE_FMT_FLTP:

                    for (size_t nb=0;nb<plane_size/sizeof(float);nb++){
                        for (int ch = 0; ch < ctx->channels; ch++) {
                            temp[temp_write] = ((float *) frame->extended_data[ch])[nb];
                            temp_write++;
                        }
                    }
                    break;
                case AV_SAMPLE_FMT_S16:

                    for (size_t nb=0;nb<plane_size/sizeof(short);nb++){
                        temp[temp_write] = ((short *) frame->extended_data[0])[nb] * SHORTTOFL;
                        temp_write++;
                    }
                    break;
                case AV_SAMPLE_FMT_FLT:

                    for (size_t nb=0;nb<plane_size/sizeof(float);nb++){
                        temp[temp_write] = ((float *) frame->extended_data[0])[nb];
                        temp_write++;
                    }
                    break;
                case AV_SAMPLE_FMT_U8P:
                    for (size_t nb=0;nb<plane_size/sizeof(uint8_t);nb++){
                        for (int ch = 0; ch < ctx->channels; ch++) {
                            temp[temp_write] = ( ( ((uint8_t *) frame->extended_data[0])[nb] - 127) * 32768 )/ 127 ;
                            temp_write++;
                        }
                    }
                    break;
                case AV_SAMPLE_FMT_U8:
                    for (size_t nb=0;nb<plane_size/sizeof(uint8_t);nb++){
                        temp[temp_write] = ( ( ((uint8_t *) frame->extended_data[0])[nb] - 127) * 32768 )/ 127 ;
                        temp_write++;
                    }
                    break;
                default:
                   WARN(9,"PCM type not supported");
                   return false;
            }
        } else {
            WARN(5,"frame failed");
            return false;
        }

        if (!_ringbuffer->Write(temp,temp_write))
        {
            WARN(9,"Write buffer not enough!");
            return false;
        }

        av_free_packet(&packet);
    }

    return true;

}
