#ifndef TEST1_H
#define TEST1_H
#include <thread>

#include "common.h"
#include "bgpoint.h"
#include "stopwatch.h"

using namespace std;

class NodeA : public BufferGraph::Point
{
private:
public:
    NodeA() :
        BufferGraph::Point()
    {

    }

    void Run();
};

class NodeB : public BufferGraph::Point
{
private:
public:
    NodeB() :
        BufferGraph::Point()
    {

    }

    void Run();
};



class Test1
{
private:
    NodeA a;
    NodeB b;
public:
    Test1();
    ~Test1();
};

#endif // TEST1_H
