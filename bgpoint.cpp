#include <thread>
#include "bgpoint.h"

Buffer *BufferGraph::Point::AcquireBuffer()
{

    Buffer *b=nullptr;
    _free_buffers->PopBlocking(b);
    assert(b);


    return b;
}

void BufferGraph::Point::ReleaseBuffer(Buffer *buffer)
{
    lock.lock();

    _buffer_refs[buffer->GetId()]++;
    if (_buffer_refs[buffer->GetId()] == _sinks.size())
    {
        _used_buffers->PopBlocking(buffer);
        //atomic_thread_fence(memory_order_seq_cst);
        _free_buffers->PushBlocking(buffer);

        _buffer_refs[buffer->GetId()]=0;
    }

    lock.unlock();
}

void BufferGraph::Point::PushBuffer(Buffer *buffer)
{
    //lock.lock();
    _used_buffers->PushBlocking(buffer);
   // lock.unlock();
}

Buffer *BufferGraph::Point::PeakBuffer()
{
    // multiple reads from more than one thread
    // since this is only a read operation
    // it is assumed to be reentrant

    // PeakBuffer() is not thread safe with
    // respect to ReleaseBuffer() any thread can
    // be releasing a buffer while another peaks
    // therefore a lock is used
    lock.lock();
    Buffer *b=nullptr;
    _used_buffers->PeakBlocking(b);
    lock.unlock();

    //   __sync_synchronize ();
    return b;
}

Buffer **BufferGraph::Point::PeakAllSources()
{
    //cout<<this_thread::get_id()<<" "<<this<<"peak"<<endl;
    _buffer_peak_update = 0;

    while (_buffer_peak_update < _sources.size()){
        for (int i=0;i<_sources.size();i++)
        {

            auto b=_sources[i]->PeakBuffer();


            if (_buffers_on_peak[i] != b)
            {
                _buffer_peak_update++;
                _buffers_on_peak[i]=b;
            } else {
                // we got the same buffer back
                // give the readers another turn
                // to get that buffer and check
                // later
                this_thread::yield();
            }
        }

    }
    _buffer_peak_update = 0;
    return _buffers_on_peak;

}

void BufferGraph::Point::ReleaseAllSources(Buffer **buffers_on_peak)
{
    for (int i=0;i<_sources.size();i++)
    {
        _sources[i]->ReleaseBuffer(buffers_on_peak[i]);
    }
}



void BufferGraph::Point::RegisterSink(Point *p)
{
    _sinks.push_back(p);
}

void BufferGraph::Point::RegisterSource(Point *p)
{
    _sources.push_back(p);

}

void BufferGraph::Point::Preallocate()
{
    _buffers_on_peak = new Buffer*[_sources.size()];

    for (int i=0;i<_sources.size();i++)
    {
        _buffers_on_peak[i]=nullptr;
    }
    _buffer_peak_update=0;
}
