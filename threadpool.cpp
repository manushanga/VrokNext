#include "threadpool.h"

ThreadPool::ThreadPool(int threadCount) :
    threadCount_(threadCount)
{
    for (int i=0;
         i<threadCount;
         i++)
    {
        threads_.push_back(thread());
    }
}

void ThreadPool::ScheduleWork(function<void (void *)> func,
                              ThreadPool::SchedMode schedMode,
                              ThreadPool::WakeUpMode wakeUpMode)
{

}

