#ifndef TEST2_H
#define TEST2_H
#include <thread>

#include "common.h"
#include "bgpoint.h"
#include "stopwatch.h"

using namespace std;

class NodeAT2 : public BufferGraph::Point
{
private:
    StopWatch *_stop_watch;
public:
    NodeAT2() :
        BufferGraph::Point()
    {

    }

    void Run();
    void SetStopWatch(StopWatch *stop_watch)
    {
        _stop_watch=stop_watch;
    }
};

class NodeBT2 : public BufferGraph::Point
{
private:
    StopWatch *_stop_watch;
public:
    NodeBT2() :
        BufferGraph::Point()
    {

    }

    void Run();
    void SetStopWatch(StopWatch *stop_watch)
    {
        _stop_watch=stop_watch;
    }
};

class NodeCT2 : public BufferGraph::Point
{
private:
    StopWatch *_stop_watch;
public:
    NodeCT2() :
        BufferGraph::Point()
    {

    }

    void Run();
    void SetStopWatch(StopWatch *stop_watch)
    {
        _stop_watch=stop_watch;
    }
};



class Test2
{
private:
    NodeAT2 a;
    NodeBT2 b;
    NodeCT2 c;
public:
    Test2();
    ~Test2();
};

#endif // TEST2_H
