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
#ifndef BUFFER_H
#define BUFFER_H
#include "bufferconfig.h"

class Buffer
{
private:
    BufferConfig _config;
    float *_buffer;
    int _size;
    int _id;

public:
    Buffer(BufferConfig& config, int id) :
        _id(id)
    {
        _config= config;
        _size=config.channels*config.frames;
        _buffer = new float[_size];

    }

    BufferConfig *GetBufferConfig()
    {
        return &_config;
    }
    void Reset(BufferConfig* config)
    {
        _config = *config;

        if (_size != _config.frames * _config.channels)
        {
            _size=_config.channels*_config.frames;
            delete[] _buffer;
            _buffer = new float[_size];
        }
    }
    int GetId()
    {
        return _id;
    }
    float *GetData()
    {
        return _buffer;
    }

};

#endif // BUFFER_H
