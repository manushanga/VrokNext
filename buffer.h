/** Buffer
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

#include "bufferconfig.h"
#include "stopwatch.h"

class Buffer
{
public:
    enum class Type : int
    {
        StreamEnd = 0,
        StreamStart,
        StreamStop,
        StreamBuffer
    };
private:
    BufferConfig _config;
    real_t *_buffer;
    int _size;
    int _id;
    Type _type;
    StopWatch _watch;
    uint64_t _stream_id;
public:
    Buffer(BufferConfig& config, int id) :
        _id(id),
        _stream_id(0UL)
    {
        _config= config;
        _size=config.channels*config.frames;
        _buffer = new real_t[_size];
        _type = Type::StreamBuffer;
    }

    void SetBufferType(Type type)
    {
        _type = type;
    }

    Type getBufferType()
    {
        return _type;
    }
    BufferConfig *GetBufferConfig()
    {
        return &_config;
    }
    void Reset(BufferConfig* config)
    {
        _config = *config;

        if (_size < _config.frames * _config.channels)
        {
            delete[] _buffer;
            _buffer = new real_t[_size];
        }
        _size=_config.channels*_config.frames;
    }
    int GetId()
    {
        return _id;
    }
    real_t *GetData()
    {
        return _buffer;
    }
    StopWatch& GetWatch()
    {
        return _watch;
    }

    void SetStreamId(uint64_t stream_id)
    {
        _stream_id = stream_id;
    }
    uint64_t GetStreamId()
    {
        return _stream_id;
    }

};
