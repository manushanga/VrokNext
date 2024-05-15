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
#include "ffmpeg.h"
#include "buffer.h"
#include "metadata.h"
#include <cstdint>
#include <limits>

#define RET_ON_ERR(ret)                                                                                      \
    if (ret < 0) {                                                                                           \
        DBG(0, "die on " << __LINE__);                                                                       \
        return false;                                                                                        \
    }
#define SHORTTOFL (1.0f / 32768.0f)
#define INT8TOFL (1.0f / 127.0f)
#define INT32TOFL (1.0f / 2147483647.0f)
#define INT64TOFL (1.0f / 9223372036854775807.0f)
#define SEEK_MAX 0xFFFFFFFFFFFFFFFFL

vrok::DecoderFFMPEG::DecoderFFMPEG() : ctx(nullptr), fmt_ctx(nullptr) {
    static long s = 0;
    if (s == 0) {
        avformat_network_init();
        // av_register_all();
    }
    s++;
    fmt_ctx = avformat_alloc_context();
    _ringbuffer = new Ringbuffer<real_t>(2 * FFMPEG_MAX_BUF_SIZE + 2 * AV_INPUT_BUFFER_PADDING_SIZE);
    _done = false;
}

vrok::DecoderFFMPEG::~DecoderFFMPEG() {
    Close();
    delete _ringbuffer;
}

bool vrok::DecoderFFMPEG::Open(vrok::Resource *resource) {
    fmt_ctx = nullptr;
    ctx = nullptr;
    _ringbuffer->Clear();

    audio_stream_id = -1;
    metadata_stream_id = -1;
    if (avformat_open_input(&fmt_ctx, resource->_filename.c_str(), NULL, NULL) < 0) {
        WARN(9, "Can't open file " << resource->_filename);
        Close();
        return false;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        WARN(9, "Stream info load failed");
        return false;
    }
    INFO("number of streams: " << fmt_ctx->nb_streams);
    unsigned int i;
    for (i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_id = i;
        } else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_DATA) {
            INFO("meta stream found!");
            metadata_stream_id = i;
        }
    }
    if (audio_stream_id == -1) {
        WARN(9, "No audio stream");
        Close();
        return false;
    }

    av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);

    audio_st = fmt_ctx->streams[audio_stream_id];
    // ctx=container->streams[audio_stream_id]->codec;
    ctx = avcodec_alloc_context3(codec);
    av_opt_set_int(ctx, "refcounted_frames", 0, 0);
    // codec=avcodec_find_decoder(ctx->codec_id);

    DBG(1, "codec: " << codec->long_name);
    if (codec == NULL) {
        WARN(9, "Cannot find codec");
        Close();
        return false;
    }

    if (avcodec_parameters_to_context(ctx, fmt_ctx->streams[audio_stream_id]->codecpar) != 0) {
        // Something went wrong. Cleaning up...
        avcodec_close(ctx);
        avcodec_free_context(&ctx);
        avformat_close_input(&fmt_ctx);
        return false;
    }

    if (avcodec_open2(ctx, codec, NULL) < 0) {
        WARN(9, "Codec cannot be opened");
        Close();
        return false;
    }

    sfmt = ctx->sample_fmt;

    duration_in_seconds = fmt_ctx->duration / AV_TIME_BASE;

    got_frame = 0;
    temp_write = 0;

    packet = av_packet_alloc();
    frame = av_frame_alloc();

    current_in_seconds = 0;
    DBG(1, "opend");
    _done = false;
    _seek_req = false;
    return true;
}

bool vrok::DecoderFFMPEG::GetBufferConfig(BufferConfig *config) {
    config->channels = audio_st->codecpar->ch_layout.nb_channels;
    config->samplerate = ctx->sample_rate;
    DBG(1, "p " << config->channels);

    DBG(1, "p " << config->samplerate);

    return true;
}

bool vrok::DecoderFFMPEG::Close() {
    if (ctx)
        avcodec_close(ctx);
    ctx = nullptr;

    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        avformat_free_context(fmt_ctx);
    }
    fmt_ctx = nullptr;

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
bool vrok::DecoderFFMPEG::DecoderRun(Buffer *buffer, BufferConfig *config) {
    /* when done drain the ring buffer */
    if (_done) {
        int used = _ringbuffer->Used();
        int bufsize = config->channels * config->frames;
        DBG(1, "used ringbuffer " << used);
        if (used == 0) {
            return false;
        } else if (used < bufsize) {
            _ringbuffer->Read(buffer->GetData(), used);
            for (int i = used; i < bufsize; i++) {
                buffer->GetData()[i] = 0.0;
            }
            return true;
        } else {
            _ringbuffer->Read(buffer->GetData(), bufsize);
            return true;
        }
    }
    if (_seek_req) {
        DBG(1, "do seek");
        int ret = avformat_seek_file(fmt_ctx, -1, INT64_MIN, _seek_to, INT64_MAX, 0);
        if (ret < 0) {
            WARN(0, "seek failed");
            return false;
        }
        _seek_req = false;
    }
    while (!_ringbuffer->Read(buffer->GetData(), config->channels * config->frames)) {
        int ret = 0, read_ok = 0, read_frame_done = 0;
        got_frame = 0;

        while (got_frame == 0) {
            if (read_frame_done == 0) {
                read_ok = av_read_frame(fmt_ctx, packet);

                if (read_ok < 0) {
                    _done = true;
                    return true;
                }
                read_frame_done = packet->stream_index == audio_stream_id;

                // eat up video packets!
                while (packet->stream_index != audio_stream_id) {
                    av_packet_unref(packet);
                    ret = av_read_frame(fmt_ctx, packet);

                    if (ret < 0) {
                        _done = true;
                        return true;
                    }
                }
                if (fmt_ctx->event_flags == AVFMT_EVENT_FLAG_METADATA_UPDATED) {
                    AVDictionaryEntry *tag = nullptr;
                    Metadata *metadata = Metadata::Create();
                    while (tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)) {
                        metadata->SetMetadata(tag->key, tag->value);
                    }
                    _metadata.push_back(metadata);

                    fmt_ctx->event_flags = 0;
                }

                read_frame_done = 1;
            }

            if (read_frame_done == 1) {
                ret = avcodec_send_packet(ctx, packet);

                av_packet_unref(packet);
                RET_ON_ERR(ret);
                read_frame_done = 0;
            }

            ret = avcodec_receive_frame(ctx, frame);

            got_frame = (ret == 0);
            RET_ON_ERR(ret);
        }
        int nb_channels = audio_st->codecpar->ch_layout.nb_channels;
        ret = av_samples_get_buffer_size(&plane_size, nb_channels, frame->nb_samples, ctx->sample_fmt, 1);
        RET_ON_ERR(ret);

        temp_write = 0;

        if (got_frame) {
            current_in_seconds = (audio_st->time_base.num * frame->pts) / audio_st->time_base.den *
                                 audio_st->time_base.num;

            switch (sfmt) {

            case AV_SAMPLE_FMT_S16P:

                for (size_t nb = 0; nb < plane_size / sizeof(int16_t); nb++) {
                    for (int ch = 0; ch < nb_channels; ch++) {
                        temp[temp_write] = ((short *)frame->extended_data[ch])[nb] * SHORTTOFL;
                        temp_write++;
                    }
                }
                break;
            case AV_SAMPLE_FMT_S32P:

                for (size_t nb = 0; nb < plane_size / sizeof(int32_t); nb++) {
                    for (int ch = 0; ch < nb_channels; ch++) {
                        temp[temp_write] = ((int32_t *)frame->extended_data[ch])[nb] * INT32TOFL;
                        temp_write++;
                    }
                }
                break;
            case AV_SAMPLE_FMT_S64P:

                for (size_t nb = 0; nb < plane_size / sizeof(int64_t); nb++) {
                    for (int ch = 0; ch < nb_channels; ch++) {
                        temp[temp_write] = ((int64_t *)frame->extended_data[ch])[nb] * INT64TOFL;
                        temp_write++;
                    }
                }
                break;
            case AV_SAMPLE_FMT_FLTP:

                for (size_t nb = 0; nb < plane_size / sizeof(float); nb++) {
                    for (int ch = 0; ch < nb_channels; ch++) {
                        temp[temp_write] = ((float *)frame->extended_data[ch])[nb];
                        temp_write++;
                    }
                }
                break;
            case AV_SAMPLE_FMT_DBLP:

                for (size_t nb = 0; nb < plane_size / sizeof(double); nb++) {
                    for (int ch = 0; ch < nb_channels; ch++) {
                        temp[temp_write] = ((double *)frame->extended_data[ch])[nb];
                        temp_write++;
                    }
                }
                break;
            case AV_SAMPLE_FMT_S16:

                for (size_t nb = 0; nb < plane_size / sizeof(short); nb++) {
                    temp[temp_write] = ((short *)frame->extended_data[0])[nb] * SHORTTOFL;
                    temp_write++;
                }
                break;
            case AV_SAMPLE_FMT_S32:

                for (size_t nb = 0; nb < plane_size / sizeof(int32_t); nb++) {
                    temp[temp_write] = ((int32_t *)frame->extended_data[0])[nb] * INT32TOFL;
                    temp_write++;
                }
                break;
            case AV_SAMPLE_FMT_S64:

                for (size_t nb = 0; nb < plane_size / sizeof(int64_t); nb++) {
                    temp[temp_write] = ((int64_t *)frame->extended_data[0])[nb] * INT64TOFL;
                    temp_write++;
                }
                break;
            case AV_SAMPLE_FMT_FLT:

                for (size_t nb = 0; nb < plane_size / sizeof(float); nb++) {
                    temp[temp_write] = ((float *)frame->extended_data[0])[nb];
                    temp_write++;
                }
                break;
            case AV_SAMPLE_FMT_DBL:

                for (size_t nb = 0; nb < plane_size / sizeof(double); nb++) {
                    temp[temp_write] = ((double *)frame->extended_data[0])[nb];
                    temp_write++;
                }
                break;
            case AV_SAMPLE_FMT_U8P:

                for (size_t nb = 0; nb < plane_size / sizeof(uint8_t); nb++) {
                    for (int ch = 0; ch < nb_channels; ch++) {
                        temp[temp_write] = ((((uint8_t *)frame->extended_data[ch])[nb] - 127)) * INT8TOFL;
                        temp_write++;
                    }
                }
                break;
            case AV_SAMPLE_FMT_U8:

                for (size_t nb = 0; nb < plane_size / sizeof(uint8_t); nb++) {
                    temp[temp_write] = ((((uint8_t *)frame->extended_data[0])[nb] - 127)) * INT8TOFL;
                    temp_write++;
                }
                break;
            default: {
                WARN(9, "PCM type not supported : " << av_get_sample_fmt_name(sfmt));
                return false;
            }
            }
        } else {

            WARN(5, "frame failed");
            return false;
        }

        if (!_ringbuffer->Write(temp, temp_write)) {
            WARN(9, "Write buffer not enough!");
            return false;
        }
        av_frame_unref(frame);
        //av_packet_unref(packet);
    }

    return true;
}

uint64_t vrok::DecoderFFMPEG::GetDurationInSeconds() {
    return duration_in_seconds;
}

uint64_t vrok::DecoderFFMPEG::GetPositionInSeconds() {
    return current_in_seconds;
}

void vrok::DecoderFFMPEG::SetPositionInSeconds(uint64_t seconds) {
    _seek_req = true;
    _seek_to = (int64_t)AV_TIME_BASE * seconds;
}

vrok::Metadata *vrok::DecoderFFMPEG::PopMetadataEvent() {
    if (!_metadata.empty()) {
        Metadata *metadata = _metadata.front();
        _metadata.pop_front();
        return metadata;
    }
    return nullptr;
}
