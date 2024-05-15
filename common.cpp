#include <thread>
#include <time.h>
#include <unistd.h>

#include "common.h"

void vrok::Sleep(int microseconds) {
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}
