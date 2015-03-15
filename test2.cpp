#include "test2.h"

Test2::Test2()
{
    a.RegisterSink(&b);
    b.RegisterSource(&a);

    c.RegisterSource(&a);
    c.RegisterSource(&b);
    a.RegisterSink(&c);
    b.RegisterSink(&c);

    a.Preallocate();
    b.Preallocate();
    c.Preallocate();

    StopWatch stop_watch;
    a.SetStopWatch(&stop_watch);
    b.SetStopWatch(&stop_watch);
    c.SetStopWatch(&stop_watch);

    a.CreateThread();
    b.CreateThread();
    c.CreateThread();
    a.JoinThread();
    b.JoinThread();
    c.JoinThread();
}

Test2::~Test2()
{

}

void NodeAT2::Run()
{
    int j=0;
    while (true)
    {

        auto b=(AcquireBuffer());

        auto xx=GetBufferConfig();

        DBG(::this_thread::get_id()<<" w"<<b);

        b->GetData()[0]=j;
        for (int x=1;x<xx->frames*xx->channels;x++)
        {
            b->GetData()[x]=x;

        }
        atomic_thread_fence(memory_order_seq_cst);
        b->GetData()[1]=1;


        j++;
        atomic_thread_fence(memory_order_seq_cst);

        PushBuffer(b);
        _stop_watch->Reset();

    }
}

void NodeBT2::Run()
{
    int j=0;
    while (true)
    {
        auto b=AcquireBuffer();
        auto xx=GetBufferConfig();
        auto bb=PeakAllSources();
        DBG(::this_thread::get_id()<<" "<<bb[0]->GetData()[1]);


        if (j!=bb[0]->GetData()[0]) {
            std::cout<<"fail"<<endl;
            while (1) {}
        }
        j++;

        for (int x=1;x<xx->frames*xx->channels;x++)
        {
            b->GetData()[x]=bb[0]->GetData()[x]-x;
        }

        atomic_thread_fence(memory_order_seq_cst);
        PushBuffer(b);

        atomic_thread_fence(memory_order_seq_cst);
        ReleaseAllSources(bb);

    }
}

void NodeCT2::Run()
{
    int j=0;
    while (true)
    {
        auto bb=PeakAllSources();
        DBG("TIME"<<_stop_watch->Stop());

        DBG(::this_thread::get_id()<<" "<<bb[0]->GetData()[1]);

        auto xx=GetBufferConfig();

        if (j!=bb[0]->GetData()[0]) {
            std::cout<<"fail"<<endl;
            while (1) {}
        }
        j++;

        for (int x=1;x<xx->frames*xx->channels;x++)
        {
            if (bb[0]->GetData()[x]!=x || bb[1]->GetData()[x] != 0)
            {
               std::cout<<"fail"<<endl;
               while (1) {}
            }
        }

        atomic_thread_fence(memory_order_seq_cst);

        ReleaseAllSources(bb);

    }
}
