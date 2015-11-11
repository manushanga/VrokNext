/** DelayBuffer (adds delay to a buffer)
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

#include <stdlib.h>
#include <cstring>
template<typename T>
class DelayBuffer
{
private:
    T *_buffer;
    int _buffer_size;
public:
    DelayBuffer(int buffer_size)  :
        _buffer(new T[buffer_size * sizeof(T)]),
        _buffer_size(buffer_size)
    {
        memset(_buffer,0,buffer_size *sizeof(T));
    }
    void FillDelayed(T *in_buffer, T *out_buffer, int delay)
    {
        for (int i=0;i<delay;i++)
        {
            out_buffer[i]=_buffer[i];
            _buffer[i]=in_buffer[_buffer_size - delay + i];

        }
        for (int i=delay;i<_buffer_size;i++)
        {
            out_buffer[i]=in_buffer[i-delay];
        }
    }
    void MixDelayed(T *in_buffer, T *out_buffer, int delay, float gain)
    {
        for (int i=0;i<delay;i++)
        {
            out_buffer[i]+=_buffer[i]*gain;
            out_buffer[i]*=0.5;
            _buffer[i]=in_buffer[_buffer_size - delay + i];

        }
        for (int i=delay;i<_buffer_size;i++)
        {
            out_buffer[i]+=in_buffer[i-delay]*gain;
            out_buffer[i]*=0.5;
        }
    }
    ~DelayBuffer()
    {
        delete [] _buffer;
    }
};

