/** BufferGraph
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

#include <atomic>
#include <thread>

#include "buffer.h"
#include "bufferconfig.h"
#include "common.h"
#include "queue.h"
#include "runnable.h"

#define BUFFERS 3

namespace BufferGraph {

class Point : public Runnable {
private:
    std::atomic<int> _ref_counter;
    std::vector<Buffer *> _buffers;
    Queue<Buffer *> *_free_buffers;
    std::mutex _lock;
    const int _max_retries = 50;

protected:
    std::vector<Point *> _sinks, _sources;
    size_t _buffer_refs[BUFFERS];
    Queue<Buffer *> *_used_buffers;
    Buffer **_buffers_on_peak;
    BufferConfig _config;
    std::atomic<uint64_t> _cur_stream_id;

public:
    Point() {
        _cur_stream_id = 0UL;

        auto _config = BufferConfig();

        _free_buffers = new Queue<Buffer *>(BUFFERS + 1);
        _used_buffers = new Queue<Buffer *>(BUFFERS + 1);

        for (int i = 0; i < BUFFERS; i++) {
            _buffer_refs[i] = 0;
            auto b = new Buffer(_config, i);
            _free_buffers->PushBlocking(b);
            _buffers.push_back(b);
        }
    }

    void SetBufferConfig(const BufferConfig *config) { _config = *config; }
    BufferConfig *GetBufferConfig() { return &_config; }
    // reentrant
    // releases a buffer owned by self when called by
    // another Point object (do not call manually, use
    // ReleaseAllSources()
    void ReleaseBuffer(Buffer *buffer);

    // reentrant (assumed) -- not with ReleaseBuffer()
    // returns what's on the top of the queue (oldest
    // unprocessed buffer). It can return the same
    // buffer for until all sinks have released buffer
    // this is resolved in PeakAllSources() use that
    // instead of this for easier interfacing
    Buffer *PeakBuffer();

    // use these functions after
    void RegisterSink(Point *p);
    void RegisterSource(Point *p);

    // preallocates all resources before the BufferGraph
    // starts rolling, must call after registering sinks
    // and sources
    void Preallocate();

    // do work on the node, this should be called from
    // a thread dedicated to the node
    // Runnable::Run()

    void Flush();

protected:
    // non reentrant
    // retrieves a buffer from the released buffer
    Buffer *AcquireBuffer();

    // non reentrant
    // push the filled buffer you acquired by calling
    // AcquireBuffer()
    void PushBuffer(Buffer *buffer);

    // non reentrant
    // one thread should call the owned Point object's
    // version of this function
    Buffer **PeakAllSources();

    // non reentrant
    // never call other objects' version
    void ReleaseAllSources(Buffer **buffers_on_peak);
};

}
