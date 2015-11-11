#pragma once

#include <mutex>
#include <iostream>
extern std::mutex __global_console_lock;


#ifdef __ANDROID_API__
#ifdef DEBUG
#include <android/log.h>
#include <sstream>
#define DBG(...) \
    {\
        stringstream ss; \
        __global_console_lock.lock(); \
        ss<<__VA_ARGS__; \
        __android_log_print(ANDROID_LOG_INFO,"vrok","%s",ss.str().c_str()); \
        __global_console_lock.unlock(); \
    }

#else
#define DBG(...)
#endif

#define WARN(...) \
    {\
        stringstream ss; \
        __global_console_lock.lock(); \
        ss<<__VA_ARGS__; \
        __android_log_print(ANDROID_LOG_INFO,"vrok","%s",ss.str().c_str()); \
        __global_console_lock.unlock(); \
    }


#else
// -- debug
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
// -- warn
#define WARN(...) \
    {\
        __global_console_lock.lock(); \
        std::cerr<<__VA_ARGS__<<std::endl; \
        __global_console_lock.unlock(); \
    }

    
#endif
