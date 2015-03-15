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
#define SHORTTOFL (1.0f/32768.0f)
#define SEEK_MAX 0xFFFFFFFFFFFFFFFFL

Vrok::DecoderFFMPEG::DecoderFFMPEG() :
    container(NULL),
    ctx(NULL)
{
    static long s=0;
    if (s==0) {
        avformat_network_init();
        av_register_all();
    }
    s++;
    container=avformat_alloc_context();
    _ringbuffer = new Ringbuffer<float>(
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

    if(avformat_open_input(&container,resource->_filename.c_str(),NULL,NULL)<0){
        DBG("Can't open file");
        return false;
    }

    if(avformat_find_stream_info(container, NULL)<0){
        DBG("Stream info load failed");
        return false;
    }

    int i;
    for(i=0;i<container->nb_streams;i++){
        if(container->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
            audio_stream_id=i;
            break;
        }
    }
    if(audio_stream_id==-1){
        DBG("No audio stream");
        return false;
    }

    audio_st = container->streams[audio_stream_id];
    ctx=container->streams[audio_stream_id]->codec;
    codec=avcodec_find_decoder(ctx->codec_id);

    DBG("codec: "<<codec->long_name);
    if(codec==NULL){
        DBG("Cannot find codec");
        return false;
    }

    if(avcodec_open2(ctx,codec,NULL)<0){
        DBG("Codec cannot be opened");
        return false;
    }

    sfmt=ctx->sample_fmt;


    duration_in_seconds = container->duration / AV_TIME_BASE;

    frameFinished=0;
    packetFinished=0;
    plane_size;
    vpbuffer_write=0;
    temp_write=0;
    remainder_read=0;
    remainder_counter=0;

    av_init_packet(&packet);
    frame=av_frame_alloc();

    current_in_seconds=0;
    DBG("opend");
    return true;
}

bool Vrok::DecoderFFMPEG::SetBufferConfig(BufferConfig *config)
{
    config->channels = ctx->channels;
    config->samplerate=ctx->sample_rate;
    DBG("p "<<config->channels);

    DBG("p "<<config->samplerate);

    return true;
}

bool Vrok::DecoderFFMPEG::Close()
{
    if (ctx)
        avcodec_close(ctx);
    if (container) {
        avformat_close_input(&container);
        avformat_free_context(container);
    }

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
    if (!_ringbuffer->Read(buffer->GetData(),config->channels*config->frames))
    {
        packetFinished = av_read_frame(container,&packet);

        // eat up video packets!
        while (packet.stream_index!=audio_stream_id && packetFinished >=0)
        {
            av_free_packet(&packet);
            packetFinished = av_read_frame(container,&packet);
        }

        if (packetFinished < 0)
            return false;

        // process audio packets
        int g=avcodec_decode_audio4(ctx,frame,&frameFinished,&packet);

        av_samples_get_buffer_size(&plane_size, ctx->channels,
                                            frame->nb_samples,
                                            ctx->sample_fmt, 1);

        temp_write=0;

        if(frameFinished){
            current_in_seconds = ( audio_st->time_base.num * frame->pkt_pts )/ audio_st->time_base.den ;
            switch (sfmt){

                case AV_SAMPLE_FMT_S16P:
                    for (int nb=0;nb<plane_size/sizeof(uint16_t);nb++){
                        for (int ch = 0; ch < ctx->channels; ch++) {
                            temp[temp_write] = ((short *) frame->extended_data[ch])[nb] * SHORTTOFL;
                            temp_write++;
                        }
                    }
                    break;
                case AV_SAMPLE_FMT_FLTP:

                    for (int nb=0;nb<plane_size/sizeof(float);nb++){
                        for (int ch = 0; ch < ctx->channels; ch++) {
                            temp[temp_write] = ((float *) frame->extended_data[ch])[nb];
                            temp_write++;
                        }
                    }
                    break;
                case AV_SAMPLE_FMT_S16:

                    for (int nb=0;nb<plane_size/sizeof(short);nb++){
                        temp[temp_write] = ((short *) frame->extended_data[0])[nb] * SHORTTOFL;
                        temp_write++;
                    }
                    break;
                case AV_SAMPLE_FMT_FLT:

                    for (int nb=0;nb<plane_size/sizeof(float);nb++){
                        temp[temp_write] = ((float *) frame->extended_data[0])[nb];
                        temp_write++;
                    }
                    break;
                case AV_SAMPLE_FMT_U8P:
                    for (int nb=0;nb<plane_size/sizeof(uint8_t);nb++){
                        for (int ch = 0; ch < ctx->channels; ch++) {
                            temp[temp_write] = ( ( ((uint8_t *) frame->extended_data[0])[nb] - 127) * 32768 )/ 127 ;
                            temp_write++;
                        }
                    }
                    break;
                case AV_SAMPLE_FMT_U8:
                    for (int nb=0;nb<plane_size/sizeof(uint8_t);nb++){
                        temp[temp_write] = ( ( ((uint8_t *) frame->extended_data[0])[nb] - 127) * 32768 )/ 127 ;
                        temp_write++;
                    }
                    break;
                default:
                   DBG("PCM type not supported");
            }
        } else {
            DBG("frame failed");
        }

        if (!_ringbuffer->Write(temp,temp_write))
        {
            DBG("Write buffer not enough!");
        }

        _ringbuffer->Read(buffer->GetData(),config->channels*config->frames);

        av_free_packet(&packet);

    }

    return true;

}
