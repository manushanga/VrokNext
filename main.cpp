
#include <queue.h>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include "bgpoint.h"
#include <atomic>

#include "test1.h"
#include "test2.h"

#include "common.h"
#include "player.h"
#include "audioout.h"
#include "preamp.h"
#include "fir.h"
#define BUF_SIZE 100


using namespace std;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::high_resolution_clock;

Queue<int> aa(2);
float time_diff(high_resolution_clock::time_point& tp1,
                high_resolution_clock::time_point& tp2)
{
    return (duration_cast<duration<float>>(tp1 - tp2)).count();
}
void push()
{
    float speed,time=0;
    std::chrono::high_resolution_clock::time_point newTime, oldTime;
    int i=0,j=0;
    while (1)
    {

        oldTime = std::chrono::high_resolution_clock::now();

        while (!aa.Push(i)) {}
        cout<<i<<endl;
        newTime = std::chrono::high_resolution_clock::now();
        if (time>1)
        {
            speed=j/time;
            j=0;
            time=0;
            cout<<"PUSH "<<speed<<endl;
        }
        else
        {
            time+=time_diff(newTime, oldTime);
            j++;
        }
        i++;
    }
}

void pop()
{
    float speed,time=0;
    std::chrono::high_resolution_clock::time_point newTime, oldTime;
    int i=0,j=0,prev=1000;

    while (!aa.Pop(prev)) {}
    while (1)
    {

        oldTime = std::chrono::high_resolution_clock::now();
        while (!aa.Pop(i)) {}
        newTime = std::chrono::high_resolution_clock::now();

        if (i!=prev+1)
        {
            cout<<"ERROR popped "<<i<<" prev is "<<prev<<endl;
            getchar();
        }
        if (time>1)
        {
            speed=j/time;
            j=0;
            time=0;
            cout<<"POP "<<speed<<endl;
        }
        else
        {
            time+=time_diff(newTime, oldTime);
            j++;
        }
        prev=i;
      //
    }
}
mutex printl,kk;
Queue<float *> free_buffers(5), used_buffers(10);
std::chrono::high_resolution_clock::time_point tp_normal;
class ClassC :public BufferGraph::Point
{
public:
    ClassC() :
        BufferGraph::Point()
    {

    }
    void Run()
    {


        int j=0;
        while (1)
        {
            //atomic_thread_fence(memory_order_acquire);

            auto bb=PeakAllSources();
            //printl.lock();
            //std::cout<<i<<" "<<((bb[0])->GetData()[0] + (bb[1])->GetData()[0])<<std::endl;
            high_resolution_clock::time_point tp=high_resolution_clock::now();
            //float nowtime=time_diff(tp,tp_normal);
            //std::cout<<nowtime<<endl;

            //atomic_thread_fence(memory_order_release);

            ReleaseAllSources(bb);

        }




    }
};

int main(int argc, char *argv[])

{
    //Test1 test;


    Vrok::Resource *res=new Vrok::Resource;
    res->_filename = std::string(argv[1]);
    Vrok::Player pl;
    Vrok::DriverAudioOut out;
    Vrok::EffectFIR pre;
    //Vrok::EffectFIR pre1;

    out.RegisterSource(&pre);
    pre.RegisterSource(&pl);
    pre.RegisterSink(&out);
    pl.RegisterSink(&pre);
    //pre1.RegisterSource(&pre);
   // pre1.RegisterSink(&out);

    out.Preallocate();
    pl.Preallocate();
    pre.Preallocate();
    //pre1.Preallocate();

    pl.SubmitForPlayback(res);
    float g=7.0;
    auto c=Vrok::ComponentManager::GetSingleton()->GetComponent("FIR filter_0");
    DBG(c);
    Vrok::ComponentManager::GetSingleton()->SetProperty(c,"dist",&g);

    pre.CreateThread();
   // pre1.CreateThread();
    out.CreateThread();

    pl.CreateThread();
    pl.JoinThread();
    pre.JoinThread();
   // pre1.JoinThread();
    out.JoinThread();

    return 0;
}
