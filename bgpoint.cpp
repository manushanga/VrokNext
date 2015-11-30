#include <thread>
#include <cstring>

#include "bgpoint.h"

Buffer *BufferGraph::Point::AcquireBuffer()
{

    Buffer *b=nullptr;

    _free_buffers->PopBlocking(b);
    return b;
}

void BufferGraph::Point::ReleaseBuffer(Buffer *buffer)
{
    lock.lock();

    _buffer_refs[buffer->GetId()]++;
    if (_buffer_refs[buffer->GetId()] == _sinks.size())
    {
        Buffer *xbuffer=nullptr;
        _used_buffers->PopBlocking(xbuffer);
        assert(xbuffer == buffer);
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
    //_buffer_peak_update = 0;

    Buffer* buffers_on_peak[_sources.size()];
    size_t buffer_peak_update=0;

    memcpy(&buffers_on_peak[0],_buffers_on_peak,sizeof(Buffer*)*_sources.size());

    int j=0;
    while (buffer_peak_update < _sources.size() && j < _max_retries){
        for (size_t i=0;i<_sources.size();i++)
        {
            auto b=_sources[i]->PeakBuffer();

            if (b == nullptr)
            {
                j=_max_retries;
                break;
            } else if ((b && b->GetStreamId() < _cur_stream_id))
            {
                std::cout<<"drop"<<b->GetStreamId() <<" "<<_cur_stream_id<<std::endl;
                _sources[i]->ReleaseBuffer(b);
            } else if (buffers_on_peak[i] != b)
            {
                buffer_peak_update++;
                buffers_on_peak[i]=b;
            } else {
                // we got the same buffer back
                // give the readers another turn
                // to get that buffer and check
                // later
                this_thread::yield();
            }
        }
        j++;
    }
    if (j < _max_retries)
    {
        memcpy(_buffers_on_peak,&buffers_on_peak[0],sizeof(Buffer*)*_sources.size());
        return _buffers_on_peak;
    } else
    {
        return nullptr;
    }

}

void BufferGraph::Point::ReleaseAllSources(Buffer **buffers_on_peak)
{
    for (size_t i=0;i<_sources.size();i++)
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

    for (size_t i=0;i<_sources.size();i++)
    {
        _buffers_on_peak[i]=nullptr;
    }
}

void BufferGraph::Point::Flush()
{
    _cur_stream_id++;
}
