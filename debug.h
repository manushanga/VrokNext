#ifndef DEBUG_H
#define DEBUG_
#include <mutex>
#include <iostream>
extern std::mutex __global_console_lock;
#ifdef DEBUG

#define DBG(...) \
    {\
        __global_console_lock.lock(); \
        std::cout<<__VA_ARGS__<<std::endl; \
        __global_console_lock.unlock(); \
    }

#else
#define DBG(...)
#endif

#define WARN(...) \
    {\
        __global_console_lock.lock(); \
        std::cerr<<__VA_ARGS__<<std::endl; \
        __global_console_lock.unlock(); \
    }

#endif // DEBUG_H

