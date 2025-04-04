/** FFmpeg wrapper
 * Copyright (C) 2013 - 2015 Madura A.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111-1307, USA.
 *
 */

#pragma once

#include <ctime>
#include <deque>
#include <iostream>

#include "debug.h"

#include "decoder.h"
#include "ringbuffer.h"

namespace vrok {

struct FFMPEGContext;

class DecoderFFMPEG : public Decoder {
public:
    static DecoderFFMPEG *Create() { return new DecoderFFMPEG(); }
    DecoderFFMPEG();
    ~DecoderFFMPEG();
    bool Open(Resource *resource);
    bool GetBufferConfig(BufferConfig *config);
    bool Close();
    bool DecoderRun(Buffer *buffer, BufferConfig *config);
    bool Play() { return true; }
    bool Pause() { return true; }
    bool Stop() { return true; }

    uint64_t GetDurationInSeconds() override;

    uint64_t GetPositionInSeconds() override;

    void SetPositionInSeconds(uint64_t seconds) override;

    Metadata *PopMetadataEvent() override;

private:
    std::deque<Metadata *> _metadata;
    bool _done;
    std::atomic<bool> _seek_req;
    int64_t _seek_to;
    Ringbuffer<real_t> *_ringbuffer;

    time_t last_read;
    int metadata_stream_id;
    int audio_stream_id;

    FFMPEGContext *fctx;
    uint64_t current_in_seconds;
    uint64_t duration_in_seconds;

    int got_frame;
    int plane_size;
    int temp_write;

    vrok::ComponentType ComponentType() { return vrok::ComponentType::Decoder; }
    Component *CreateSelf() { return new DecoderFFMPEG(); }
    const char *ComponentName() { return "FFmpeg Decoder"; }
    const char *Description() { return "FFmpeg wrapper"; }
    const char *Author() { return "Madura A."; }
    const char *License() { return "GPL v2"; }
};
}
