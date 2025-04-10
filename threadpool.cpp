#include "threadpool.h"
#include "common.h"
#include "debug.h"
#include "util/mutil.h"

using namespace std;

// interval in usec
#define TIMEOUT 10000
vrok::ThreadPool::ThreadPool(size_t thread_count) {

    _runnables.resize(thread_count);
    _threads.resize(thread_count);
    _thread_data.resize(thread_count);
}

bool vrok::ThreadPool::RegisterWork(size_t thread_id, Runnable *runnable) {
    if (thread_id < _runnables.size()) {
        _runnables[thread_id].push_back(runnable);
        return true;
    } else {
        return false;
    }
}
void vrok::ThreadPool::CreateThreads() {
    for (size_t i = 0; i < _threads.size(); i++) {
        ThreadData *th = new ThreadData;
        th->runnables = &_runnables;
        th->thread_id = i;
        th->work = true;

        _thread_data[i] = th;

        _threads[i] = new thread(ThreadPool::Work, th);
    }
}
void vrok::ThreadPool::StopThreads() {
    for (size_t i = 0; i < _threads.size(); i++) {
        _thread_data[i]->work = false;
        _threads[i]->join();
    }
}

void vrok::ThreadPool::JoinThreads() {
    for (size_t i = 0; i < _threads.size(); i++) {
        _threads[i]->join();
    }
}

vrok::ThreadPool::~ThreadPool() {
    for (size_t i = 0; i < _threads.size(); i++) {
        delete _threads[i];
        delete _thread_data[i];
    }
}

void vrok::ThreadPool::Work(ThreadData *th) {
    std::stringstream sstr;
    sstr << "vrok:" << th->thread_id;
    __set_thread_name(sstr.str());

    for (size_t i = 0; i < (*th->runnables)[th->thread_id].size(); i++) {
        // DBG(th->thread_id<<" "<<i);
        (*th->runnables)[th->thread_id][i]->ThreadStart();
    }

    while (th->work) {
        for (size_t i = 0; i < (*th->runnables)[th->thread_id].size(); i++) {
            auto start = std::chrono::steady_clock::now();
            (*th->runnables)[th->thread_id][i]->Run();
            auto end = std::chrono::steady_clock::now();

            long time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            if (time < TIMEOUT) {
                vrok::Sleep((int)(TIMEOUT - time));
            }
        }
    }

    for (size_t i = 0; i < (*th->runnables)[th->thread_id].size(); i++) {
        // DBG(th->thread_id<<" "<<i);
        (*th->runnables)[th->thread_id][i]->ThreadEnd();
    }
    INFO("in use on thread:" << th->thread_id << " " << mutil_get_in_use() << " bytes");
}
