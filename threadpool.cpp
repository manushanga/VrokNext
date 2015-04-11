#include "threadpool.h"
#include "debug.h"
ThreadPool::ThreadPool(int thread_count)
{

    _runnables.resize(thread_count);
    _threads.resize(thread_count);
    _thread_data.resize(thread_count);
}

bool ThreadPool::RegisterWork(int thread_id, Runnable *runnable)
{
    if (thread_id < _runnables.size())
    {
        _runnables[thread_id].push_back(runnable);
        return true;
    } else
    {
        return false;
    }
}

void ThreadPool::CreateThreads()
{
    for (int i=0;
         i<_threads.size();
         i++)
    {
        ThreadData *th=new ThreadData;
        th->runnables = &_runnables;
        th->thread_id = i;
        th->work = true;

        _thread_data[i] = th;

        _threads[i] =new thread(ThreadPool::Work, th);
    }
}

void ThreadPool::JoinThreads()
{
    for (int i=0;
         i<_threads.size();
         i++)
    {
        _threads[i]->join();
    }
}


ThreadPool::~ThreadPool()
{
    for (int i=0;i<_threads.size();i++)
    {
        delete _threads[i];
        delete _thread_data[i];
    }
}

void ThreadPool::Work(ThreadData *th)
{

    while (th->work)
    {
        for (int i=0;i<(*th->runnables)[th->thread_id].size();i++)
        {
            DBG(th->thread_id<<" "<<i);
            (*th->runnables)[th->thread_id][i]->Run();
        }
    }
}

