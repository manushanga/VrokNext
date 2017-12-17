#pragma once

#include <mutex>
#include <iostream>
#include <sstream>

#include "notify.h"

extern std::mutex __global_console_lock;


#ifdef __ANDROID_API__
#include <android/log.h>

std::string __get_thread_name();
void __set_thread_name(std::string thread_name);

#ifdef DEBUG
#define DBG(__level, ...) \
    {\
        std::stringstream ss; \
        __global_console_lock.lock(); \
        ss<<__get_thread_name()<<": "<<__VA_ARGS__; \
        __android_log_print(ANDROID_LOG_INFO,"vrok","%s",ss.str().c_str()); \
        __global_console_lock.unlock(); \
    }

#else
#define DBG(...)
#endif

#define WARN(__level, ...) \
    {\
        std::stringstream ss; \
        __global_console_lock.lock(); \
        ss<<__get_thread_name()<<": "<<__VA_ARGS__; \
        __android_log_print(ANDROID_LOG_INFO,"vrok","%s",ss.str().c_str()); \
        __global_console_lock.unlock(); \
    }


#else
// -- debug
#ifdef DEBUG

#define DBG(__level, ...) \
    {\
        std::stringstream __sstream; \
        __sstream << __VA_ARGS__ ; \
        Vrok::Notify::GetInstance()->NotifyDebug(__level, __sstream.str()); \
    }

#else
#define DBG(...)
#endif
// -- warn
#define WARN(__level, ...) \
    {\
        std::stringstream __sstream; \
        __sstream << __VA_ARGS__ ; \
        Vrok::Notify::GetInstance()->NotifyWarning(__level, __sstream.str()); \
    }

    
#endif
