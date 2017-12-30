#pragma once

class Runnable
{
public:
    virtual ~Runnable() {}
    virtual void ThreadStart() {}
    virtual void Run() = 0;
    virtual void ThreadEnd() {}
};


