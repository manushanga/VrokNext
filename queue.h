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
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <chrono>
#include <thread>

#include "common.h"
#include "debug.h"

void wait_for(bool &condition, bool wait_while, std::unique_lock<std::mutex> &lock,
              std::condition_variable &cv, unsigned int microseconds);

template <typename T>
class Queue {
private:
    std::atomic<int> _front, _rear;
    int _g_front, _g_rear;
    int _size;
    T *_container;
    std::mutex _guard;
    std::atomic<bool> _bpop, _bpush;

    std::mutex _lock_on_empty;
    std::mutex _lock_on_full;
    std::condition_variable _cv_empty;
    std::condition_variable _cv_full;

    bool _is_empty;
    bool _is_full;

public:
    Queue(int size) : _size(size), _bpop(false), _bpush(false) {
        _g_front = _front = 0;
        _g_rear = _rear = 0;

        _container = static_cast<T *>(malloc(sizeof(T) * size));
    }
    int GetSize() const { return _size; }
    bool PopLocked(T &t) {
        std::lock_guard<std::mutex> ll(_guard);
        if (_g_rear == _g_front) {
            return false;
        } else {
            t = _container[_g_rear];
            _g_rear = (_g_rear + 1) % _size;
            return true;
        }
    }
    bool PeakLocked(T &t) {
        std::lock_guard<std::mutex> ll(_guard);
        if (_g_rear == _g_front) {
            return false;
        } else {
            t = _container[_g_rear];
            return true;
        }
    }
    bool PushLocked(T t) {
        std::lock_guard<std::mutex> ll(_guard);
        int nfront = (_g_front + 1) % _size;
        if (nfront == _g_rear) {
            return false;
        } else {
            _container[_g_front] = t;
            _g_front = nfront;
            return true;
        }
    }
    bool Peak(T &t) {
        int cr = _rear.load(std::memory_order_relaxed);
        if (cr == _front.load(std::memory_order_acquire)) {
            return false;
        }

        t = _container[cr];
        return true;
    }
    bool Pop(T &t) {
        int cr = _rear.load(std::memory_order_relaxed);

        if (cr == _front.load(std::memory_order_acquire)) {
            return false;
        }
        t = _container[cr];
        _rear.store((cr + 1) % _size, std::memory_order_release);
        return true;
    }
    bool Push(T t) {
        int cf = _front.load(std::memory_order_relaxed);

        int new_cf = (cf + 1) % _size;
        if (new_cf == _rear.load(std::memory_order_acquire)) {

            return false;
        }

        _container[cf] = t;
        _front.store(new_cf, std::memory_order_release);

        return true;
    }
    bool PeakBlocking(T &t) {
        if (Peak(t) == false) {
            std::unique_lock<std::mutex> lk(_lock_on_empty);
            _is_empty = false;
            wait_for(_is_empty, false, lk, _cv_empty, 100);
        }

        return true;
    }
    bool PopBlocking(T &t) {
        bool ret = Pop(t);

        if (ret) {
            {
                std::unique_lock<std::mutex> lk(_lock_on_full);
                _is_full = false;
            }
            _cv_full.notify_all();
        } else {
            std::unique_lock<std::mutex> lk(_lock_on_empty);
            _is_empty = false;
            wait_for(_is_empty, false, lk, _cv_empty, 100);
        }

        return true;
    }
    bool PushBlocking(T t) {
        bool ret = Push(t);

        if (ret) {
            {
                std::unique_lock<std::mutex> lk(_lock_on_empty);
                _is_empty = true;
            }
            _cv_empty.notify_all();
        } else {
            std::unique_lock<std::mutex> lk(_lock_on_full);
            _is_full = true;
            wait_for(_is_full, true, lk, _cv_full, 100);
        }

        return true;
    }

    void PrintQueue() {
        for (int i = 0; i < _size; i++) {
            std::cout << _container[i] << " ";
        }
        std::cout << std::endl;
    }

    ~Queue() { free(_container); }

private:
};

#endif // QUEUE_H
