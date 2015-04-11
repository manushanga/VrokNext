#ifndef DEBUG_H
#define DEBUG_
#include <mutex>
extern std::mutex __global_console_lock;
#ifdef DEBUG
#include <iostream>
#define DBG(...) \
    {\
        __global_console_lock.lock(); \
        std::cout<<__VA_ARGS__<<std::endl; \
        __global_console_lock.unlock(); \
    }

#else
#define DBG(...)
#endif

#endif // DEBUG_H

