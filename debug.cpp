#include <mutex>
#include <string>
#ifdef __APPLE__
#include <pthread.h>
#else
#include <sys/prctl.h>
#endif

std::mutex __global_console_lock;

std::string __get_thread_name() {
    char name[64] = {};
#ifdef __APPLE__
    pthread_getname_np(pthread_self(), name, sizeof(name));
#else
    prctl(PR_GET_NAME, name, 0, 0, 0);
#endif
    return std::string(name);
}

void __set_thread_name(std::string thread_name) {
#ifdef __APPLE__
    pthread_setname_np(thread_name.substr(0, 63).c_str());
#else
    prctl(PR_SET_NAME, (unsigned long)thread_name.c_str(), 0, 0, 0);
#endif
}
