/** Lock-free, Blocking, Locked Queue
 * Based on,
 * http://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular
 * https://0xdeafc0de.wordpress.com/2015/01/11/a-queue-implementation-using-atomic-instructions-with-benchmarks/
 *
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
#ifndef QUEUE_H
#define QUEUE_H
#include <atomic>
#include <vector>
#include <condition_variable>
#include <iostream>
#include <cstdlib>

#include <thread>
#include <chrono>

#include "debug.h"

template<typename T>
class Queue
{
private:
    alignas(64) std::atomic<int> _front,_rear;
    alignas(64) int _g_front, _g_rear;
    int _size;
    T *_container;
    std::mutex _guard;
    std::atomic<bool> _bpop,_bpush;
    const int _max_tries=500;
    // sleep for some amount of micro seconds
    const int _sleep_for=1000;
public:
    Queue(int size) :
        _size(size),
        _bpop(false),
        _bpush(false)
    {
        _g_front = _front = 0;
        _g_rear = _rear = 0;

        _container = static_cast<T *>( malloc(sizeof(T)*size) );

    }
    int GetSize() const
    {
        return _size;
    }
    bool PopLocked(T& t)
    {
        std::lock_guard<std::mutex> ll(_guard);
        if (_g_rear == _g_front)
        {
            return false;
        } else {
            t= _container[_g_rear];
            _g_rear=(_g_rear+1)%_size;
            return true;
        }

    }
    bool PeakLocked(T& t)
    {
        std::lock_guard<std::mutex> ll(_guard);
        if (_g_rear == _g_front)
        {
            return false;
        } else {
            t= _container[_g_rear];
            return true;
        }

    }
    bool PushLocked(T t)
    {
        std::lock_guard<std::mutex> ll(_guard);
        int nfront=(_g_front + 1)%_size;
        if (nfront == _g_rear)
        {
            return false;
        } else {
            _container[_g_front]=t;
            _g_front=nfront;
            return true;
        }
    }
    bool Peak(T& t)
    {
        int cr=_rear.load(std::memory_order_relaxed);
        if (cr == _front.load(std::memory_order_acquire))
        {
            return false;
        }

        t=_container[cr];
        return true;

    }
    bool Pop(T& t)
    {
        int cr=_rear.load(std::memory_order_relaxed);
        //std::cout<<cr<<" "<<cf<<std::endl;
        if (cr == _front.load(std::memory_order_acquire))
        {
            return false;
        }
        t = _container[cr];
        _rear.store((cr+1)%_size,std::memory_order_release);
        return true;


    }
    bool Push(T t)
    {
        int cf=_front.load(std::memory_order_relaxed);

        int new_cf=(cf+1) % _size;
        if (new_cf == _rear.load(std::memory_order_acquire)) {

            return false;
        }

        _container[cf]=t;
        _front.store(new_cf,std::memory_order_release);

        return true;
    }
    bool PeakBlocking(T& t)
    {
        int i=0;
        while (!Peak(t) && i<_max_tries) { i++; std::this_thread::sleep_for(std::chrono::microseconds(_sleep_for +1)); }
#ifdef DEBUG
        if (i==max_tries) DBG(6,"drop");
#endif
        return i<_max_tries;
    }
    bool PopBlocking(T& t)
    {
        int i=0;
        while (!Pop(t) && i<_max_tries) {  i++; std::this_thread::sleep_for(std::chrono::microseconds(_sleep_for));  }
#ifdef DEBUG
        if (i==max_tries) DBG(6,"drop");
#endif
        return i<_max_tries;
 
    }
    bool PushBlocking(T t)
    {
        int i=0;
        while (!Push(t) && i<_max_tries) {  i++; std::this_thread::sleep_for(std::chrono::microseconds(_sleep_for -1)); }
#ifdef DEBUG
        if (i==max_tries) DBG(6,"drop");
#endif
        return i<_max_tries;
    }


    void PrintQueue()
    {
        for (int i=0;i<_size;i++)
        {
            std::cout<<_container[i]<<" ";
        }
        std::cout<<std::endl;
    }

    ~Queue()
    {
        free(_container);
    }
};

#endif // QUEUE_H
