#include <thread>
#include <unistd.h>
#include <time.h>

#include "common.h"

void Vrok::Sleep(int microseconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

