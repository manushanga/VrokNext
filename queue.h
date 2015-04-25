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

using namespace std;
template<typename T>
class Queue
{
private:
    alignas(64) atomic<int> front_,rear_;
    alignas(64) int g_front_, g_rear_;
    int size_;
    T *container;
    mutex guard;
    atomic<bool> bpop_,bpush_;
    condition_variable cv_pop_,cv_push_;
    const int max_tries=200;
public:
    Queue(int size) :
        size_(size),
        bpop_(false),
        bpush_(false)
    {
        g_front_ = front_ = 0;
        g_rear_ = rear_ = 0;

        container = static_cast<T *>( malloc(sizeof(T)*size) );

    }
    int GetSize() const
    {
        return size_;
    }
    bool PopLocked(T& t)
    {
        lock_guard<mutex> ll(guard);
        if (g_rear_ == g_front_)
        {
            return false;
        } else {
            t= container[g_rear_];
            g_rear_=(g_rear_+1)%size_;
            return true;
        }

    }
    bool PeakLocked(T& t)
    {
        lock_guard<mutex> ll(guard);
        if (g_rear_ == g_front_)
        {
            return false;
        } else {
            t= container[g_rear_];
            return true;
        }

    }
    bool PushLocked(T t)
    {
        lock_guard<mutex> ll(guard);
        int nfront=(g_front_ + 1)%size_;
        if (nfront == g_rear_)
        {
            return false;
        } else {
            container[g_front_]=t;
            g_front_=nfront;
            return true;
        }
    }
    bool Peak(T& t)
    {
        int cr=rear_.load(memory_order_relaxed);
        if (cr == front_.load(memory_order_acquire))
        {
            return false;
        }

        t=container[cr];
        return true;

    }
    bool Pop(T& t)
    {
        int cr=rear_.load(memory_order_relaxed);
        //std::cout<<cr<<" "<<cf<<std::endl;
        if (cr == front_.load(memory_order_acquire))
        {
            return false;
        }
        t = container[cr];
        rear_.store((cr+1)%size_,memory_order_release);
        return true;


    }
    bool Push(T t)
    {
        int cf=front_.load(memory_order_relaxed);

        int new_cf=(cf+1) % size_;
        if (new_cf == rear_.load(memory_order_acquire)) {

            return false;
        }

        container[cf]=t;
        front_.store(new_cf,memory_order_release);

        return true;
    }
    bool PeakBlocking(T& t)
    {
        int i=0;
        while (!Peak(t) && i<max_tries) { i++; this_thread::sleep_for(chrono::microseconds(501)); }
        
        return i<max_tries;
    }
    bool PopBlocking(T& t)
    {
        int i=0;
        while (!Pop(t) && i<max_tries) {  i++; this_thread::sleep_for(chrono::microseconds(500));  }
        
        return i<max_tries;

    }
    bool PushBlocking(T t)
    {
        int i=0;
        while (!Push(t) && i<max_tries) {  i++; this_thread::sleep_for(chrono::microseconds(499)); }
        
        return i<max_tries;
    }


    void PrintQueue()
    {
        for (int i=0;i<size_;i++)
        {
            std::cout<<container[i]<<" ";
        }
        std::cout<<std::endl;
    }

    ~Queue()
    {
        free(container);
    }
};

#endif // QUEUE_H
