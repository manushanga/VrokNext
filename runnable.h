#ifndef RUNNABLE
#define RUNNABLE
class Runnable
{
public:
    virtual void Init() {}
    virtual void Run() = 0;
    virtual void Fini() {}
};
#endif // RUNNABLE

