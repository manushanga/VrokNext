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
#ifndef BUFFERCONFIG_H
#define BUFFERCONFIG_H

#include "common.h"

struct BufferConfig
{
    int frames,channels,samplerate;

    BufferConfig(int frames_=1024, int channels_=2, int samplerate_=48000) :
        frames(frames_),
        channels(channels_),
        samplerate(samplerate_)
    {

    }
    inline bool operator!=(const BufferConfig& lhs)
    {
        return !((lhs.channels == channels) &&
                (lhs.frames == frames) &&
                (lhs.samplerate == samplerate));
    }
    void Print()
    {
        DBG(frames<<" "<<channels<<" "<<samplerate);

    }
};
#endif // BUFFERCONFIG_H
