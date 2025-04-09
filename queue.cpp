#include "queue.h"
void wait_for(bool &condition, bool wait_while, std::unique_lock<std::mutex> &lock,
              std::condition_variable &cv, unsigned int microseconds) {
    unsigned int to_be_spent = microseconds;
    unsigned int spent = 0;
    while (condition == wait_while && spent < to_be_spent) {
        auto start = std::chrono::steady_clock::now();
        cv.wait_for(lock, std::chrono::microseconds(to_be_spent));
        auto end = std::chrono::steady_clock::now();
        spent += (unsigned int)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
}