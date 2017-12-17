#include <sys/prctl.h>
#include <mutex>

std::mutex __global_console_lock;

std::string __get_thread_name()
{
    char name[16];
    prctl(PR_GET_NAME, name, 0,0,0);
    return std::string(name);
}

void __set_thread_name(std::string thread_name)
{
    prctl(PR_SET_NAME, (unsigned long)thread_name.c_str(), 0, 0, 0);
}
