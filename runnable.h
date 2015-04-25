#ifndef RUNNABLE
#define RUNNABLE
class Runnable
{
public:
    virtual void ThreadStart() {}
    virtual void Run() = 0;
    virtual void ThreadEnd() {}
};
#endif // RUNNABLE

