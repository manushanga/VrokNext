/**
 * Copyright (C) Madura A.
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
 */

#pragma once

#include "buffer.h"
#include "bufferconfig.h"
#include "common.h"
#include "resource.h"

namespace vrok {
class Metadata;
class Decoder : public Component {
public:
    virtual ~Decoder() { }
    virtual Metadata *PopMetadataEvent() { return nullptr; }
    virtual bool Open(Resource *resource) = 0;
    virtual bool GetBufferConfig(BufferConfig *config) = 0;
    virtual bool Close() = 0;
    virtual bool Play() = 0;
    virtual bool Pause() = 0;
    virtual bool Stop() = 0;
    virtual bool DecoderRun(Buffer *buffer, BufferConfig *config) = 0;
    virtual uint64_t GetDurationInSeconds() = 0;
    virtual uint64_t GetPositionInSeconds() = 0;
    virtual void SetPositionInSeconds(uint64_t seconds) = 0;
};
}
