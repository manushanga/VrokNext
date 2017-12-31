/** RingBuffer
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

#include <cstdlib>
#include <util/mutil.h>

template<typename T>
class Ringbuffer
{
private:
    T *_buffer;
    size_t _size,_front,_rear,_used;
public:
    Ringbuffer(size_t size) :
        _size(size),
        _front(0),
        _rear(0),
        _used(0)
    {
        _buffer = (T*)mutil_aligned_alloc(size * sizeof(T));
    }
    inline bool IsWritable(size_t n) const
    {
        return (_used + n <= _size);
    }
    inline bool IsReadable(size_t n) const
    {
        return (n <= _used);
    }
    inline bool Write(T *source, size_t n)
    {
        if (_used + n <= _size)
        {
            for (size_t i=0;i<n;i++)
            {
                _buffer[(_front + i)%_size] = source[i];
            }
            _front = (_front + n) % _size;
            _used += n;
            return true;
        } else {
            return false;
        }
    }

    inline bool Read(T *dest, size_t n)
    {
        if (n <= _used)
        {
            for (size_t i=0;i<n;i++)
            {
                dest[i] = _buffer[(_rear + i)%_size];
            }
            _rear = (_rear + n) % _size;
            _used -= n;
            return true;
        } else
        {
            return false;
        }
    }
    void Clear()
    {
        _front = 0;
        _rear = 0;
        _used = 0;
    }
    std::size_t Used()
    {
        return _used;
    }
    std::size_t Size()
    {
        return _size;
    }
    ~Ringbuffer()
    {
        mutil_aligned_free(_buffer);
    }

};

