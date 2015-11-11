#pragma once

class Runnable
{
public:
    virtual void ThreadStart() {}
    virtual void Run() = 0;
    virtual void ThreadEnd() {}
};


